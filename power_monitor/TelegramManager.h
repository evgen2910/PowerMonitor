#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "Config.h"

class TelegramManager {
public:
    static TelegramManager& instance() { static TelegramManager t; return t; }

    // Додати повідомлення в чергу (thread-safe через loop)
    void enqueue(const String& text);

    // Викликати з loop() — відправляє по одному з черги
    void loop();

    // Спробувати визначити chat_id через getUpdates
    bool fetchChatId(String& outId);

    int  queueCount()     const { return _count; }
    int  totalSent()      const { return _totalSent; }
    int  totalDelivered() const { return _totalDelivered; }

    // Для Web UI: перегляд черги (read-only)
    const TgMessage& queueItem(int i) const {
        return _q[(_head + i) % MSG_QUEUE_SIZE];
    }

private:
    TelegramManager() {}

    bool _send(const String& text);
    String _apiPath(const String& method) const;
    String _readResponse(WiFiClientSecure& cl, uint32_t timeoutMs = 10000);

    TgMessage _q[MSG_QUEUE_SIZE];
    int  _head  = 0;
    int  _tail  = 0;
    int  _count = 0;

    int  _totalSent      = 0;
    int  _totalDelivered = 0;

    uint32_t _lastAttempt = 0;
};

inline TelegramManager& Tg() { return TelegramManager::instance(); }
