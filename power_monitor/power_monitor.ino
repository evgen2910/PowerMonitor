/*
 * ESP32 Power Monitor v5.0
 * ============================================================
 * Підтримувані плати:
 *   ESP32 classic (WROOM / WROVER / DevKit)  — за замовчуванням
 *   ESP32-C3 SuperMini / XIAO / DevKitM-1   — #define BOARD_ESP32C3
 *   ESP32-S2                                 — #define BOARD_ESP32S2
 *   ESP32-S3                                 — #define BOARD_ESP32S3
 *
 * Для ESP32-C3 SuperMini в Arduino IDE:
 *   Board: "ESP32C3 Dev Module"
 *   USB CDC On Boot: Enabled  (для Serial через USB)
 *   Partition Scheme: Default 4MB
 *   Розкоментуй: #define BOARD_ESP32C3  в Config.h
 * ============================================================
 * Файли: Config, Logger, TimeUtils, WiFiManager,
 *        TelegramManager, PowerMonitor, OtaManager, WebUI
 * Залежності: тільки ESP32 Arduino Core
 * ============================================================
 */

#include "Config.h"
#include "Logger.h"
#include "TimeUtils.h"
#include "WiFiManager.h"
#include "TelegramManager.h"
#include "PowerMonitor.h"
#include "PowerEvent.h"
#include "OtaManager.h"
#include "WebUI.h"

// ---------------------------------------------------------------
//  LED heartbeat (враховує active-low для C3 SuperMini)
// ---------------------------------------------------------------
static void updateLed() {
    static uint32_t last = 0;
    static bool ledState = false;
    uint32_t interval = WM().isConnected()  ? STATUS_BLINK_CONNECTED_MS :
                        WM().isCaptive()    ? STATUS_BLINK_PORTAL_MS    :
                                              500;
    if (millis() - last > interval) {
        last = millis();
        ledState = !ledState;
#if LED_ACTIVE_LOW
        digitalWrite(LED_PIN, ledState ? LOW : HIGH);
#else
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
#endif
    }
}

// ---------------------------------------------------------------
//  NTP авто-ресинхронізація
// ---------------------------------------------------------------
static void checkNtp() {
    if (!WM().isConnected()) return;
    if (Time().isSynced() &&
        millis() - Time().lastSyncMs() < NTP_SYNC_INTERVAL_MS) return;
    Time().sync();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n\n=== ESP32 Power Monitor v" FW_VERSION " [" BOARD_NAME "] ===");

    pinMode(LED_PIN, OUTPUT);
#if LED_ACTIVE_LOW
    digitalWrite(LED_PIN, HIGH);  // вимкнено (active-low)
#else
    digitalWrite(LED_PIN, HIGH);
#endif

    // 1. Конфігурація з NVS
    Cfg().load();

    // 2. GPIO моніторингу
    PM().begin();

    // 3. WiFi (спробувати підключитися; якщо ні → captive)
    WM().begin();

    // 4. Web сервер
    UI().begin();

    // 5. NTP — вже синхронізується в WM().begin() при підключенні.
    //    Якщо не синхронізовано (captive portal) — спробуємо ще раз коли з'явиться WiFi.

    // 6. Перевірка збереженої події відключення
    if (WM().isConnected() && PwrEvt().hasPendingOutage() && PM().powerPresent()) {
        // Живлення є зараз, але є збережена незакрита подія — значить відновилось
        String offAt    = PwrEvt().offTimeStr();
        uint32_t durSec = PwrEvt().onPowerRestored();
        String ts       = Time().now();

        Log().add("✅ [старт] Живлення відновлено після відключення о " + offAt);

        if (Cfg().cfg.notify_power_on) {
            String msg  = "✅ *ЖИВЛЕННЯ ВІДНОВЛЕНО*\n";
            msg += "📛 " + String(Cfg().cfg.device_name) + "\n";
            msg += "❌ Зникло о: " + offAt + "\n";
            msg += "✅ Відновлено: " + ts + "\n";
            if (durSec > 0)
                msg += "⏱ Тривалість: " + Time().durationSec(durSec) + "\n";
            if (Cfg().cfg.batt_enabled)
                msg += "🔋 Батарея: " + String(PM().battVoltage(), 2) + " В  (" + String(PM().battPercent()) + "%)\n";
            msg += "_(виявлено після перезавантаження)_";
            Tg().enqueue(msg);
        }
    }

    // 7. Привітальне повідомлення в Telegram
    if (WM().isConnected() && Cfg().cfg.notify_boot) {
        String msg  = "🔌 *" + String(Cfg().cfg.device_name) + "* — старт\n";
        msg += "📶 WiFi: "  + WM().ssid()   + "\n";
        msg += "🌐 IP: "    + WM().ip()     + "\n";
        msg += "⚡ Живлення: " + String(PM().powerPresent() ? "✅ Присутнє" : "❌ Відсутнє") + "\n";
        if (Cfg().cfg.batt_enabled)
            msg += "🔋 Батарея: " + String(PM().battVoltage(), 2) + " В  (" + String(PM().battPercent()) + "%)\n";
        msg += "🔢 Версія: " FW_VERSION "\n";
        msg += "🕐 " + Time().now();
        Tg().enqueue(msg);
    }

    Log().add("Система запущена v" FW_VERSION " [" BOARD_NAME "]");
#if LED_ACTIVE_LOW
    digitalWrite(LED_PIN, LOW);   // вмикаємо (active-low) — потім updateLed() керує
#else
    digitalWrite(LED_PIN, LOW);
#endif
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
    WM().loop();         // WiFi reconnect / DNS captive
    UI().loop();         // HTTP сервер
    PM().loop();         // GPIO, антидребезг, події → Tg()
    Tg().loop();         // Черга Telegram
    checkNtp();          // NTP ресинхронізація
    updateLed();         // LED heartbeat

    delay(5);
}
