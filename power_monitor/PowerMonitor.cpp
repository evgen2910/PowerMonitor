#include "PowerMonitor.h"
#include "Config.h"
#include "Logger.h"
#include "TimeUtils.h"
#include "TelegramManager.h"
#include "PowerEvent.h"
#include <time.h>

// ---------------------------------------------------------------
void PowerMonitor::applyPinConfig() {
    const Config& c = Cfg().cfg;
    uint8_t mode;
    switch (c.power_pin_mode) {
        case 1:  mode = INPUT_PULLUP;   break;
        case 2:  mode = INPUT_PULLDOWN; break;
        default: mode = INPUT;          break;
    }
    pinMode(c.power_pin, mode);
    Serial.printf("[PM] GPIO%d mode=%d inv=%d board=%d\n",
                  c.power_pin, c.power_pin_mode, (int)c.power_active_low, c.board_type);

    // Налаштування ADC для батареї
    if (c.batt_enabled) {
        pinMode(c.batt_pin, INPUT);
        analogReadResolution(12);
        // board_type: 0=ESP32 classic, 1=C3, 2=S2, 3=S3
        // ESP32 classic — глобальна атенюація; інші — per-pin
        if (c.board_type == 0) {
            analogSetAttenuation(ADC_11db);
        } else {
            analogSetPinAttenuation(c.batt_pin, ADC_11db);
        }
        Serial.printf("[PM] Батарея: GPIO%d R1=%.0fk R2=%.0fk %dS full=%.1fV empty=%.1fV\n",
                      c.batt_pin, c.batt_r1_kohm, c.batt_r2_kohm,
                      c.batt_cells, c.batt_full_v, c.batt_empty_v);
    }
}

// ---------------------------------------------------------------
void PowerMonitor::begin() {
    applyPinConfig();

    // Завантажити стан подій з NVS (перед перевіркою поточного стану)
    PwrEvt().begin();

    _power   = _readPin();
    _lastRaw = _power;
    if (_power) {
        _powerOnAt  = millis();
        _powerOffAt = 0;
    } else {
        _powerOffAt = millis();
        _powerOnAt  = 0;
    }
    if (Cfg().cfg.batt_enabled) {
        _lastBattUpd = 0;
        _updateBattery();
    }
    Serial.printf("[PM] Живлення: %s\n", _power ? "ПРИСУТНЄ" : "ВІДСУТНЄ");
}

// ---------------------------------------------------------------
bool PowerMonitor::_readPin() const {
    bool raw = digitalRead(Cfg().cfg.power_pin);
    return Cfg().cfg.power_active_low ? !raw : raw;
}

bool PowerMonitor::battEnabled() const {
    return Cfg().cfg.batt_enabled;
}

// ---------------------------------------------------------------
void PowerMonitor::_updateBattery() {
    const Config& c = Cfg().cfg;
    if (!c.batt_enabled) return;

    // Усереднення BATT_ADC_SAMPLES вимірів
    // analogReadMilliVolts() використовує вбудовану фабричну калібровку ESP32
    // і повертає реальні мВ на піні — точніше ніж ручний розрахунок через VREF
    uint32_t sumMv = 0;
    for (int i = 0; i < BATT_ADC_SAMPLES; i++) {
        sumMv += analogReadMilliVolts(c.batt_pin);
        delayMicroseconds(200);
    }
    float vPinMv = (float)sumMv / BATT_ADC_SAMPLES;
    float vPin   = vPinMv / 1000.0f;  // мВ → В

    // Реальна напруга батареї через дільник: Vbatt = Vpin * (R1+R2) / R2
    float vBatt = vPin * (c.batt_r1_kohm + c.batt_r2_kohm) / c.batt_r2_kohm;
    _battV = vBatt;

    // Відсоток заряду (лінійна інтерполяція)
    float range = c.batt_full_v - c.batt_empty_v;
    int pct = 0;
    if (range > 0.01f) {
        pct = (int)(((vBatt - c.batt_empty_v) / range) * 100.0f);
        pct = constrain(pct, 0, 100);
    }
    _battPct = pct;

    Serial.printf("[PM] Batt: pin=%dmV vPin=%.3fV vBatt=%.2fV %d%%\n",
                  (int)vPinMv, vPin, vBatt, _battPct);
    bool wasLow = _battLow;
    _battLow = (pct <= BATT_LOW_THRESHOLD);

    // Сповіщення: тільки при першому переході в "низький"
    if (_battLow && !wasLow && !_battLowSent && c.notify_batt_low) {
        _battLowSent = true;
        String msg = "🔋 *БАТАРЕЯ РОЗРЯДЖАЄТЬСЯ*\n";
        msg += "📛 " + String(c.device_name) + "\n";
        msg += "⚡ " + String(vBatt, 2) + " В  (" + String(pct) + "%)\n";
        msg += "🕐 " + Time().now();
        Tg().enqueue(msg);
        Log().add("Батарея низька: " + String(vBatt, 2) + "В (" + String(pct) + "%)");
    }
    // Скидаємо флаг коли зарядилась
    if (!_battLow && pct > BATT_LOW_THRESHOLD + 5) {
        _battLowSent = false;
    }
}

// ---------------------------------------------------------------
void PowerMonitor::loop() {
    const Config& c = Cfg().cfg;  // кешуємо — використовується багаторазово

    // --- Живлення мережі (антидребезг) ---
    bool current = _readPin();
    if (current != _lastRaw) {
        _lastRaw       = current;
        _pendingActive = true;
        _pendingValue  = current;
        _debounceStart = millis();
    }
    if (_pendingActive) {
        if (current != _pendingValue) {
            _pendingValue  = current;
            _debounceStart = millis();
        } else if (millis() - _debounceStart >= DEBOUNCE_MS) {
            _pendingActive = false;
            if (current != _power) {
                _power = current;
                String ts = Time().now();
                String msg;
                // Рядок стану батареї (додається до всіх повідомлень якщо увімкнено)
                auto battStr = [&]() -> String {
                    if (!c.batt_enabled) return "";
                    String s = "🔋 Батарея: " + String(_battV, 2) + " В  (" + String(_battPct) + "%)";
                    if (_battLow) s += " ⚠️";
                    return "\n" + s;
                };

                if (!_power) {
                    _powerOffAt = millis();
                    _outageCount++;
                    _lastOffTs  = Time().unixNow();
                    Log().add("❌ Живлення ЗНИКЛО о " + ts);

                    // Зберігаємо момент зникнення в NVS — виживе навіть якщо батарея сяде
                    PwrEvt().onPowerLost();

                    if (c.notify_power_off) {
                        msg  = "❌ *ЖИВЛЕННЯ ЗНИКЛО*\n";
                        msg += "📛 " + String(c.device_name) + "\n";
                        msg += "🕐 " + Time().now();
                        msg += battStr();
                        Tg().enqueue(msg);
                    }
                } else {
                    _lastOffDuration = (_powerOffAt > 0) ? (millis() - _powerOffAt) : 0;
                    _powerOnAt  = millis();
                    _powerOffAt = 0;
                    _lastOnTs   = Time().unixNow();

                    // Зберігаємо рядок часу зникнення ДО очищення
                    String offAt   = PwrEvt().offTimeStr();
                    uint32_t durSec = PwrEvt().onPowerRestored();  // очищає NVS

                    String logMsg = "✅ Живлення З'ЯВИЛОСЯ о " + ts;
                    if (durSec > 0)
                        logMsg += " (відсутнє: " + Time().durationSec(durSec) + ")";
                    else if (_lastOffDuration > 0)
                        logMsg += " (відсутнє: " + Time().duration(_lastOffDuration) + ")";
                    Log().add(logMsg);

                    if (c.notify_power_on) {
                        msg  = "✅ *ЖИВЛЕННЯ З'ЯВИЛОСЯ*\n";
                        msg += "📛 " + String(c.device_name) + "\n";
                        if (offAt.length() && offAt != "час невідомий")
                            msg += "❌ Зникло о: " + offAt + "\n";
                        msg += "✅ Відновлено: " + ts + "\n";
                        if (durSec > 0)
                            msg += "⏱ Тривалість: " + Time().durationSec(durSec);
                        else if (_lastOffDuration > 0)
                            msg += "⏱ Тривалість: " + Time().duration(_lastOffDuration);
                        msg += battStr();
                        Tg().enqueue(msg);
                    }
                }
            }
        }
    }

    // --- Батарея ---
    if (c.batt_enabled &&
        millis() - _lastBattUpd >= BATT_UPDATE_MS) {
        _lastBattUpd = millis();
        _updateBattery();
    }
}
