#pragma once
#include <Arduino.h>

class PowerMonitor {
public:
    static PowerMonitor& instance() { static PowerMonitor p; return p; }

    void begin();
    void loop();

    // --- Мережеве живлення ---
    bool     powerPresent()   const { return _power; }
    uint32_t offDurationMs()  const {
        if (_power) return 0;                    // зараз є живлення
        if (_powerOffAt == 0) return 0;          // ще не фіксували відключення
        return millis() - _powerOffAt;
    }
    uint32_t onDurationMs()   const {
        if (!_power || _powerOnAt == 0) return 0;
        return millis() - _powerOnAt;
    }
    uint32_t lastOffDurationMs() const { return _lastOffDuration; }
    uint32_t outageCount()       const { return _outageCount; }    // лічильник відключень
    time_t   lastOffTimestamp()  const { return _lastOffTs; }      // unix ts останнього відключення
    time_t   lastOnTimestamp()   const { return _lastOnTs; }       // unix ts останнього відновлення

    // --- Батарея ---
    float    battVoltage()    const { return _battV; }     // В
    int      battPercent()    const { return _battPct; }   // 0-100%
    bool     battLow()        const { return _battLow; }
    bool     battEnabled()    const;

    void applyPinConfig();

private:
    PowerMonitor() {}
    bool  _readPin() const;
    void  _updateBattery();

    // Живлення мережі
    bool     _power         = true;
    bool     _lastRaw       = true;
    bool     _pendingValue  = false;
    bool     _pendingActive = false;
    uint32_t _debounceStart = 0;
    uint32_t _powerOffAt    = 0;
    uint32_t _powerOnAt     = 0;
    uint32_t _lastOffDuration = 0;
    uint32_t _outageCount    = 0;    // лічильник відключень за сесію
    time_t   _lastOffTs      = 0;    // unix timestamp останнього зникнення
    time_t   _lastOnTs       = 0;    // unix timestamp останнього відновлення

    // Батарея
    float    _battV         = 0.0f;
    int      _battPct       = 0;
    bool     _battLow       = false;
    bool     _battLowSent   = false;   // вже надіслано сповіщення
    uint32_t _lastBattUpd   = 0;
};

inline PowerMonitor& PM() { return PowerMonitor::instance(); }
