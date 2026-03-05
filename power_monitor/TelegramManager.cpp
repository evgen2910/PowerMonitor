#include "TelegramManager.h"
#include "WiFiManager.h"
#include "Logger.h"

// ---------------------------------------------------------------
String TelegramManager::_apiPath(const String& method) const {
    return "/bot" + String(Cfg().cfg.bot_token) + "/" + method;
}

// ---------------------------------------------------------------
String TelegramManager::_readResponse(WiFiClientSecure& cl, uint32_t timeoutMs) {
    String response;
    response.reserve(512);
    bool     inBody  = false;
    uint32_t deadline = millis() + timeoutMs;
    char     buf[64];

    while ((cl.connected() || cl.available()) && millis() < deadline) {
        int avail = cl.available();
        if (avail <= 0) { delay(1); continue; }

        if (!inBody) {
            // Читаємо рядок за рядком щоб знайти кінець заголовків
            String line = cl.readStringUntil('\n');
            if (line == "\r") inBody = true;
        } else {
            // Читаємо чанками — не побайтово
            int got = cl.readBytes(buf, min(avail, (int)sizeof(buf)));
            if (got > 0) response.concat(buf, got);
        }
    }
    cl.stop();
    return response;
}

// ---------------------------------------------------------------
bool TelegramManager::_send(const String& text) {
    if (!WM().isConnected()) return false;
    if (strlen(Cfg().cfg.bot_token) == 0) return false;
    // Вибір chat_id залежно від типу чату
    const char* chatId = (Cfg().cfg.chat_type == 1)
                         ? Cfg().cfg.group_chat_id
                         : Cfg().cfg.chat_id;
    if (strlen(chatId) == 0) return false;

    WiFiClientSecure cl;
    cl.setInsecure();
    cl.setTimeout(12);

    if (!cl.connect(TG_HOST, TG_PORT)) {
        Serial.println("[TG] Помилка з'єднання");
        return false;
    }

    // Екранування для JSON
    String escaped = text;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");

    String body;
    body.reserve(escaped.length() + 64);
    body = "{\"chat_id\":\"";
    body += chatId;
    body += "\",\"text\":\"";
    body += escaped;
    body += "\",\"parse_mode\":\"Markdown\"}";

    // Надсилаємо по частинах — не будуємо весь req в одному String
    cl.print("POST ");
    cl.print(_apiPath("sendMessage"));
    cl.print(" HTTP/1.1\r\nHost: " TG_HOST "\r\nContent-Type: application/json\r\nContent-Length: ");
    cl.print(body.length());
    cl.print("\r\nConnection: close\r\n\r\n");
    cl.print(body);

    String resp = _readResponse(cl);
    bool ok = resp.indexOf("\"ok\":true") >= 0;
    if (!ok) {
        Serial.printf("[TG] Відхилено: %.120s\n", resp.c_str());
    }
    return ok;
}

// ---------------------------------------------------------------
void TelegramManager::enqueue(const String& text) {
    if (_count >= MSG_QUEUE_SIZE) {
        Serial.println("[TG] Черга переповнена!");
        return;
    }
    _q[_tail].text      = text;
    _q[_tail].addedAt   = millis();
    _q[_tail].retries   = 0;
    _q[_tail].delivered = false;
    _tail  = (_tail + 1) % MSG_QUEUE_SIZE;
    _count++;
    Serial.printf("[TG] Черга +1 (всього: %d)\n", _count);
}

// ---------------------------------------------------------------
void TelegramManager::loop() {
    if (_count == 0) return;
    if (millis() - _lastAttempt < MSG_RETRY_DELAY_MS) return;
    _lastAttempt = millis();

    TgMessage& msg = _q[_head];
    if (msg.delivered) {               // Вже доставлено, просто прибрати
        _head  = (_head + 1) % MSG_QUEUE_SIZE;
        _count--;
        return;
    }

    _totalSent++;
    bool ok = _send(msg.text);

    if (ok) {
        msg.delivered = true;
        _totalDelivered++;
        Serial.println("[TG] ✓ Доставлено");
        _head  = (_head + 1) % MSG_QUEUE_SIZE;
        _count--;
    } else {
        msg.retries++;
        Serial.printf("[TG] Спроба %d/%d\n", msg.retries, MSG_MAX_RETRIES);
        if (msg.retries >= MSG_MAX_RETRIES) {
            Log().add("TG: повідомлення відхилено після " +
                      String(MSG_MAX_RETRIES) + " спроб");
            _head  = (_head + 1) % MSG_QUEUE_SIZE;
            _count--;
        }
    }
}

// ---------------------------------------------------------------
bool TelegramManager::fetchChatId(String& outId) {
    if (!WM().isConnected()) return false;
    if (strlen(Cfg().cfg.bot_token) == 0) return false;

    WiFiClientSecure cl;
    cl.setInsecure();
    cl.setTimeout(15);

    if (!cl.connect(TG_HOST, TG_PORT)) return false;

    String req = "GET " + _apiPath("getUpdates") +
                 "?limit=5&timeout=5 HTTP/1.1\r\n"
                 "Host: " TG_HOST "\r\n"
                 "Connection: close\r\n\r\n";
    cl.print(req);

    String resp = _readResponse(cl, 12000);

    // Шукаємо "chat":{"id": ...
    int ci = resp.indexOf("\"chat\"");
    if (ci < 0) {
        // Спроба знайти просто перший "id":
        ci = resp.indexOf("\"from\"");
    }
    if (ci < 0) return false;

    int ii = resp.indexOf("\"id\":", ci);
    if (ii < 0) return false;
    ii += 5;
    // Пропустимо пробіли
    while (ii < (int)resp.length() && resp[ii] == ' ') ii++;
    int end = ii;
    // id може бути від'ємним (групи починаються з -)
    if (resp[end] == '-') end++;
    while (end < (int)resp.length() && isDigit(resp[end])) end++;
    if (end == ii) return false;

    outId = resp.substring(ii, end);
    outId.trim();

    if (outId.startsWith("-")) {
        // Це група/канал
        strlcpy(Cfg().cfg.group_chat_id, outId.c_str(), sizeof(Cfg().cfg.group_chat_id));
        Cfg().cfg.chat_type = 1;
    } else {
        // Особистий чат
        strlcpy(Cfg().cfg.chat_id, outId.c_str(), sizeof(Cfg().cfg.chat_id));
        Cfg().cfg.chat_type = 0;
    }
    return outId.length() > 0;
}
