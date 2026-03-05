#pragma once
#include <Arduino.h>

enum class OtaStatus {
    IDLE,
    IN_PROGRESS,
    SUCCESS,
    FAIL
};

class OtaManager {
public:
    static OtaManager& instance() { static OtaManager o; return o; }

    // Завантаження з URL (http або https)
    // Повертає true при успіху; прогрес → callback(bytes, total)
    bool flashFromUrl(const String& url,
                      std::function<void(size_t,size_t)> progress = nullptr);

    OtaStatus status()  const { return _status; }
    String    lastErr() const { return _lastErr; }

    // Викликається WebServer'ом для завантаження файлу
    // Реєструє endpoint /update на переданому сервері
    void handleUploadBegin();
    void handleUploadChunk(uint8_t* data, size_t len, bool last);
    void handleUploadEnd();
    bool uploadOk() const { return _uploadOk; }

private:
    OtaManager() {}
    OtaStatus _status    = OtaStatus::IDLE;
    String    _lastErr;
    bool      _uploadOk  = false;
};

inline OtaManager& OTA() { return OtaManager::instance(); }
