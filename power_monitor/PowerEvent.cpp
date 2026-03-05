#include "PowerEvent.h"
#include "TimeUtils.h"

// ---------------------------------------------------------------
void PowerEvent::begin() {
    _prefs.begin("pmevt", true);  // read-only
    _offTs  = _prefs.getUInt("off_ts", 0);
    String s = _prefs.getString("off_str", "");
    _offStr  = s;
    _prefs.end();

    if (_offTs > 0) {
        Serial.printf("[PwrEvt] Знайдено незакриту подію: зникло о %s (ts=%u)\n",
                      _offStr.c_str(), _offTs);
    } else {
        Serial.println("[PwrEvt] Попередніх незакритих подій немає");
    }
}

// ---------------------------------------------------------------
void PowerEvent::onPowerLost() {
    time_t ts = Time().unixNow();
    String str = Time().now();

    // Якщо NTP не синхронізовано — зберігаємо 1 як маркер "час невідомий"
    // (0 означає "немає події", тому використовуємо 1)
    _offTs  = (ts > 0) ? (uint32_t)ts : 1;
    _offStr = (ts > 0) ? str : "час невідомий";

    _save();
    Serial.printf("[PwrEvt] Зафіксовано зникнення: %s (ts=%u)\n",
                  _offStr.c_str(), _offTs);
}

// ---------------------------------------------------------------
uint32_t PowerEvent::onPowerRestored() {
    if (_offTs == 0) return 0;  // не було зафіксованого зникнення

    uint32_t durSec = 0;
    time_t   now    = Time().unixNow();

    if (_offTs > 1 && now > (time_t)_offTs) {
        // Обидва timestamps відомі — точна тривалість
        durSec = (uint32_t)(now - _offTs);
    }
    // Якщо _offTs==1 або now==0 — час невідомий, повертаємо 0 але подія є

    _clear();
    return durSec;
}

// ---------------------------------------------------------------
void PowerEvent::_save() {
    _prefs.begin("pmevt", false);
    _prefs.putUInt  ("off_ts",  _offTs);
    _prefs.putString("off_str", _offStr);
    _prefs.end();
}

void PowerEvent::_clear() {
    _offTs  = 0;
    _offStr = "";
    _prefs.begin("pmevt", false);
    _prefs.clear();
    _prefs.end();
    Serial.println("[PwrEvt] Подія закрита, NVS очищено");
}
