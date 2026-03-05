#pragma once
#include <Arduino.h>
#include "Config.h"

class Logger {
public:
    static Logger& instance() { static Logger l; return l; }

    void add(const String& msg);
    int  count() const { return _count; }
    // Доступ за індексом (0 = найновіший)
    const EventEntry& get(int i) const {
        int idx = (_head - 1 - i + EVENT_LOG_SIZE * 2) % EVENT_LOG_SIZE;
        return _buf[idx];
    }

private:
    Logger() {}
    EventEntry _buf[EVENT_LOG_SIZE];
    int _head  = 0;
    int _count = 0;
};

inline Logger& Log() { return Logger::instance(); }
