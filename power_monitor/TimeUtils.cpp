#include "TimeUtils.h"
#include "Config.h"

void TimeUtils::sync() {
    configTime(Cfg().cfg.tz_offset_sec, 0,
               Cfg().cfg.ntp_server,
               "time.google.com",
               "time.cloudflare.com");

    struct tm ti;
    uint8_t attempts = 0;
    while (!getLocalTime(&ti) && attempts < 20) {
        delay(300);
        attempts++;
    }
    _synced   = getLocalTime(&ti);
    _lastSync = millis();
    if (_synced) {
        char buf[32];
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &ti);
        Serial.printf("[NTP] Синхронізовано: %s\n", buf);
    } else {
        Serial.println("[NTP] Помилка синхронізації");
    }
}

String TimeUtils::now() const {
    struct tm ti;
    if (!getLocalTime(&ti)) return "н/д";
    char buf[32];
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &ti);
    return String(buf);
}

String TimeUtils::durationSec(uint32_t s) const {
    uint32_t m = s / 60; s %= 60;
    uint32_t h = m / 60; m %= 60;
    uint32_t d = h / 24; h %= 24;
    String r;
    if (d) r += String(d) + "д ";
    if (h) r += String(h) + "г ";
    if (m) r += String(m) + "хв ";
    r += String(s) + "с";
    return r;
}

time_t TimeUtils::unixNow() const {
    struct tm ti;
    if (!getLocalTime(&ti)) return 0;
    return mktime(&ti);
}

String TimeUtils::duration(uint32_t ms) const {
    uint32_t s = ms / 1000;
    uint32_t m = s / 60; s %= 60;
    uint32_t h = m / 60; m %= 60;
    uint32_t d = h / 24; h %= 24;
    String r;
    if (d) r += String(d) + "д ";
    if (h) r += String(h) + "г ";
    if (m) r += String(m) + "хв ";
    r += String(s) + "с";
    return r;
}
