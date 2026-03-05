#include "Logger.h"

void Logger::add(const String& msg) {
    Serial.printf("[LOG] %s\n", msg.c_str());
    _buf[_head].text = msg;
    _buf[_head].ts   = millis();
    _head = (_head + 1) % EVENT_LOG_SIZE;
    if (_count < EVENT_LOG_SIZE) _count++;
}
