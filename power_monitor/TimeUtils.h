#pragma once
#include <Arduino.h>
#include <time.h>

class TimeUtils {
public:
    static TimeUtils& instance() { static TimeUtils t; return t; }

    void     sync();
    bool     isSynced() const { return _synced; }
    String   now() const;                    // "dd.mm.yyyy hh:mm:ss"
    String   duration(uint32_t ms) const;    // з мілісекунд: "Xд Yг Zхв Nс"
    String   durationSec(uint32_t s) const;  // з секунд:     "Xд Yг Zхв Nс"
    time_t   unixNow() const;                // unix timestamp (0 якщо не синхронізовано)
    uint32_t lastSyncMs() const { return _lastSync; }

private:
    TimeUtils() {}
    bool     _synced   = false;
    uint32_t _lastSync = 0;
};

inline TimeUtils& Time() { return TimeUtils::instance(); }
