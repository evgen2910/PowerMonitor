#include "OtaManager.h"
#include "Logger.h"
#include <Update.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

// ---------------------------------------------------------------
// Розбирає URL на компоненти
struct UrlParts {
    bool   https;
    String host;
    int    port;
    String path;
};

static bool parseUrl(const String& url, UrlParts& out) {
    if (url.startsWith("https://")) {
        out.https = true;
        out.port  = 443;
        String rest = url.substring(8);
        int slash = rest.indexOf('/');
        out.host = (slash < 0) ? rest : rest.substring(0, slash);
        out.path = (slash < 0) ? "/" : rest.substring(slash);
    } else if (url.startsWith("http://")) {
        out.https = false;
        out.port  = 80;
        String rest = url.substring(7);
        int slash = rest.indexOf('/');
        out.host = (slash < 0) ? rest : rest.substring(0, slash);
        out.path = (slash < 0) ? "/" : rest.substring(slash);
    } else {
        return false;
    }
    // Порт у хості?
    int colon = out.host.indexOf(':');
    if (colon >= 0) {
        out.port = out.host.substring(colon + 1).toInt();
        out.host = out.host.substring(0, colon);
    }
    return true;
}

// ---------------------------------------------------------------
bool OtaManager::flashFromUrl(const String& url,
                               std::function<void(size_t,size_t)> progress) {
    _status  = OtaStatus::IN_PROGRESS;
    _lastErr = "";
    Log().add("OTA URL: " + url);

    UrlParts up;
    if (!parseUrl(url, up)) {
        _lastErr = "Невірний URL";
        _status  = OtaStatus::FAIL;
        return false;
    }

    // Клієнт (http або https)
    Client* cl = nullptr;
    WiFiClient       plain;
    WiFiClientSecure secure;

    if (up.https) {
        secure.setInsecure();
        secure.setTimeout(30);
        cl = &secure;
    } else {
        plain.setTimeout(30);
        cl = &plain;
    }

    if (!cl->connect(up.host.c_str(), up.port)) {
        _lastErr = "Не вдалося підключитися до " + up.host;
        _status  = OtaStatus::FAIL;
        Log().add("OTA помилка: " + _lastErr);
        return false;
    }

    // GET запит
    String req = "GET " + up.path + " HTTP/1.1\r\n"
                 "Host: " + up.host + "\r\n"
                 "Connection: close\r\n\r\n";
    cl->print(req);

    // Читаємо заголовки, шукаємо Content-Length
    size_t contentLength = 0;
    String line;
    uint32_t deadline = millis() + 15000;
    while (millis() < deadline) {
        line = cl->readStringUntil('\n');
        line.trim();
        if (line.isEmpty()) break;                     // кінець заголовків
        if (line.startsWith("HTTP/") && line.indexOf("200") < 0 &&
            line.indexOf("301") < 0 && line.indexOf("302") < 0) {
            _lastErr = "HTTP: " + line;
            _status  = OtaStatus::FAIL;
            cl->stop();
            return false;
        }
        String lower = line; lower.toLowerCase();
        if (lower.startsWith("content-length:")) {
            contentLength = line.substring(15).toInt();
        }
    }

    if (!Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN)) {
        _lastErr = "Update.begin помилка";
        _status  = OtaStatus::FAIL;
        cl->stop();
        return false;
    }

    size_t written  = 0;
    uint8_t buf[512];
    deadline = millis() + 120000;   // 2 хв на завантаження

    while (cl->connected() && written < contentLength && millis() < deadline) {
        size_t avail = cl->available();
        if (avail == 0) { delay(10); continue; }
        size_t toRead = min(avail, sizeof(buf));
        size_t got    = cl->read(buf, toRead);
        if (got) {
            size_t wr = Update.write(buf, got);
            written += wr;
            if (progress) progress(written, contentLength);
        }
    }
    cl->stop();

    if (Update.end(true)) {
        _status = OtaStatus::SUCCESS;
        Log().add("OTA завершено успішно (" + String(written) + " байт)");
        return true;
    } else {
        _lastErr = "Update.end помилка: " + String(Update.getError());
        _status  = OtaStatus::FAIL;
        Log().add("OTA помилка: " + _lastErr);
        return false;
    }
}

// ---------------------------------------------------------------
void OtaManager::handleUploadBegin() {
    _uploadOk = false;
    _status   = OtaStatus::IN_PROGRESS;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        _lastErr = "Update.begin помилка";
        _status  = OtaStatus::FAIL;
        Serial.println("[OTA] begin помилка");
    }
}

void OtaManager::handleUploadChunk(uint8_t* data, size_t len, bool last) {
    if (_status == OtaStatus::FAIL) return;
    if (Update.write(data, len) != len) {
        _lastErr = "Помилка запису прошивки";
        _status  = OtaStatus::FAIL;
    }
    if (last) {
        if (Update.end(true)) {
            _uploadOk = true;
            _status   = OtaStatus::SUCCESS;
            Log().add("OTA (файл) завершено успішно");
        } else {
            _lastErr = "Update.end: " + String(Update.getError());
            _status  = OtaStatus::FAIL;
        }
    }
}

void OtaManager::handleUploadEnd() {
    // Нічого — результат у _uploadOk
}
