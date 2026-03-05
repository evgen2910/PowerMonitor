#pragma once
#include <Arduino.h>
#include <Preferences.h>

// ============================================================
//  PowerEvent — відстеження відключень живлення між перезавантаженнями
//
//  Зберігає момент зникнення світла в NVS (flash) одразу при події.
//  Після перезавантаження (навіть після повного знеструмлення через батарею)
//  читає збережений timestamp і повідомляє тривалість відключення.
//
//  NVS namespace: "pmevt"  (окремо від конфігу "pm")
//  Ключі:
//    "off_ts"   — unix timestamp моменту зникнення (uint32, 0 = немає)
//    "off_str"  — рядок дати/часу зникнення ("dd.mm.yyyy hh:mm:ss")
//    "reported" — bool, чи вже надіслано повідомлення після відновлення
// ============================================================

class PowerEvent {
public:
    static PowerEvent& instance() {
        static PowerEvent inst;
        return inst;
    }

    // Викликати при зникненні живлення — зберігає в NVS
    void onPowerLost();

    // Викликати при появі живлення — повертає тривалість в секундах (0 = не було події)
    // Очищає запис після повернення
    uint32_t onPowerRestored();

    // Рядок часу зникнення (для повідомлення), порожній якщо події немає
    String   offTimeStr() const { return _offStr; }

    // Чи є незакрита подія зникнення
    bool     hasPendingOutage() const { return _offTs > 0; }

    // Завантажити стан з NVS при старті
    void     begin();

    // Unix timestamp моменту зникнення (0 якщо немає)
    uint32_t offTimestamp() const { return _offTs; }

private:
    PowerEvent() {}
    void _save();
    void _clear();

    Preferences _prefs;
    uint32_t _offTs  = 0;   // unix timestamp зникнення
    String   _offStr;       // рядок для відображення
};

inline PowerEvent& PwrEvt() { return PowerEvent::instance(); }
