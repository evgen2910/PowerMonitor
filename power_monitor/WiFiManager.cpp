#include "WiFiManager.h"
#include "Config.h"
#include "Logger.h"
#include "TimeUtils.h"
#include <ESPmDNS.h>

// ---------------------------------------------------------------
bool WiFiManager::_tryConnect(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return false;
    Serial.printf("[WiFi] Підключення до: %s\n", ssid);
    WiFi.begin(ssid, pass);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(200);
    }
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::_startSTA() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true);
    delay(100);
}

void WiFiManager::_startMDNS() {
    const char* name = Cfg().cfg.mdns_name;
    if (!name || strlen(name) == 0) return;
    MDNS.end();
    if (MDNS.begin(name)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[mDNS] http://%s.local\n", name);
    }
}

// ---------------------------------------------------------------
void WiFiManager::begin() {
    _startSTA();
    const Config& c = Cfg().cfg;
    bool ok = _tryConnect(c.wifi_ssid1, c.wifi_pass1);
    if (!ok) ok = _tryConnect(c.wifi_ssid2, c.wifi_pass2);

    if (ok) {
        _state      = WiFiState::CONNECTED;
        _retryCount = 0;
        String msg = "WiFi підключено: " + WiFi.SSID() +
                     "  IP: " + WiFi.localIP().toString();
        Log().add(msg);
        Serial.println("[WiFi] " + msg);
        _startMDNS();
        Time().sync();
    } else {
        // Не вдалося при старті — одразу Captive Portal
        Log().add("WiFi: не вдалося підключитися → Captive Portal");
        startCaptivePortal();
    }
    _lastAttempt = millis();
}

// ---------------------------------------------------------------
void WiFiManager::loop() {

    // ── CAPTIVE PORTAL ──────────────────────────────────────
    // Обслуговуємо DNS, але також пробуємо перепідключитись
    // кожні 3 хвилини — раптом роутер повернувся
    if (_state == WiFiState::CAPTIVE_PORTAL) {
        dns.processNextRequest();

        // Якщо WiFi SSID взагалі не налаштований — нема чого пробувати
        if (strlen(Cfg().cfg.wifi_ssid1) == 0 &&
            strlen(Cfg().cfg.wifi_ssid2) == 0) return;

        if (millis() - _lastAttempt < WIFI_RECONNECT_INTERVAL_MS) return;
        _lastAttempt = millis();

        Serial.println("[WiFi] Captive Portal: спроба підключення...");
        dns.stop();
        WiFi.softAPdisconnect(true);
        delay(200);

        _startSTA();
        const Config& c = Cfg().cfg;
        bool ok = _tryConnect(c.wifi_ssid1, c.wifi_pass1);
        if (!ok) ok = _tryConnect(c.wifi_ssid2, c.wifi_pass2);

        if (ok) {
            _state      = WiFiState::CONNECTED;
            _retryCount = 0;
            Log().add("WiFi підключено (з Captive Portal): " + WiFi.SSID() +
                      "  IP: " + WiFi.localIP().toString());
            _startMDNS();
            Time().sync();
        } else {
            Serial.println("[WiFi] Не вдалося, повертаємось до Captive Portal");
            startCaptivePortal();   // перезапускаємо AP+DNS
        }
        return;
    }

    // ── CONNECTED ───────────────────────────────────────────
    if (_state == WiFiState::CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            _state      = WiFiState::DISCONNECTED;
            _retryCount = 0;
            _lastAttempt = millis();
            Log().add("WiFi: з'єднання втрачено");
        }
        checkAsyncScan();
        return;
    }

    // ── DISCONNECTED ────────────────────────────────────────
    // Інтервал між спробами: перші 3 рази — 30с, потім — 3 хв
    uint32_t interval = (_retryCount < 3)
                        ? 30000UL
                        : WIFI_RECONNECT_INTERVAL_MS;

    if (millis() - _lastAttempt < interval) return;
    _lastAttempt = millis();

    Serial.printf("[WiFi] Перепідключення (спроба %d)...\n", _retryCount + 1);
    _startSTA();
    const Config& c = Cfg().cfg;
    bool ok = _tryConnect(c.wifi_ssid1, c.wifi_pass1);
    if (!ok) ok = _tryConnect(c.wifi_ssid2, c.wifi_pass2);

    if (ok) {
        _state      = WiFiState::CONNECTED;
        _retryCount = 0;
        Log().add("WiFi перепідключено: " + WiFi.SSID() +
                  "  IP: " + WiFi.localIP().toString());
        _startMDNS();
        Time().sync();
    } else {
        _retryCount++;
        Serial.printf("[WiFi] Не вдалося (%d спроб)\n", _retryCount);

        // Після 5 невдалих спроб — Captive Portal щоб можна було
        // змінити налаштування якщо SSID/пароль стали неправильними
        if (_retryCount >= 5) {
            Log().add("WiFi: " + String(_retryCount) +
                      " невдалих спроб → Captive Portal");
            startCaptivePortal();
            _retryCount = 0;
        }
    }
}

// ---------------------------------------------------------------
void WiFiManager::startCaptivePortal() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);
    dns.start(53, "*", AP_IP);
    _state = WiFiState::CAPTIVE_PORTAL;
    Serial.printf("[AP] %s / %s  IP: %s\n",
                  AP_SSID, AP_PASS, AP_IP.toString().c_str());
    Log().add("Captive Portal (SSID: " + String(AP_SSID) + ")");
}

void WiFiManager::reconnectNow() {
    if (_state == WiFiState::CAPTIVE_PORTAL) {
        dns.stop();
        WiFi.softAPdisconnect(true);
        delay(200);
    }
    _state       = WiFiState::DISCONNECTED;
    _retryCount  = 0;
    _lastAttempt = 0;   // підключитись негайно
}

void WiFiManager::restartMDNS() {
    if (_state == WiFiState::CONNECTED) _startMDNS();
}

// ---------------------------------------------------------------
//  Спільна логіка обробки результатів скану
// ---------------------------------------------------------------
void WiFiManager::_processScanResults(int n) {
    _scanCount = 0;
    if (n <= 0) { WiFi.scanDelete(); return; }

    int total = min(n, (int)WIFI_SCAN_MAX);
    struct Tmp { String ssid; int32_t rssi; uint8_t enc; };
    Tmp tmp[WIFI_SCAN_MAX];
    for (int i = 0; i < total; i++)
        tmp[i] = {WiFi.SSID(i), WiFi.RSSI(i), (uint8_t)WiFi.encryptionType(i)};
    // Сортування за спаданням RSSI (bubble sort, n ≤ WIFI_SCAN_MAX=10)
    for (int i = 0; i < total - 1; i++)
        for (int j = 0; j < total - 1 - i; j++)
            if (tmp[j].rssi < tmp[j+1].rssi)
                { auto t=tmp[j]; tmp[j]=tmp[j+1]; tmp[j+1]=t; }

    const Config& c = Cfg().cfg;
    for (int i = 0; i < total; i++) {
        _nets[i] = {tmp[i].ssid, tmp[i].rssi, tmp[i].enc,
                    (tmp[i].ssid == c.wifi_ssid1 || tmp[i].ssid == c.wifi_ssid2)};
    }
    _scanCount = total;
    WiFi.scanDelete();
}

// ---------------------------------------------------------------
//  Scan — синхронне
// ---------------------------------------------------------------
int WiFiManager::scanNetworks() {
    Serial.println("[WiFi] Сканування...");
    int n = WiFi.scanNetworks(false, true);
    _processScanResults(n);
    if (_scanCount == 0) { Serial.println("[WiFi] Мережі не знайдено"); return 0; }
    Serial.printf("[WiFi] Знайдено: %d мереж\n", _scanCount);
    return _scanCount;
}

// ---------------------------------------------------------------
//  Async scan
// ---------------------------------------------------------------
void WiFiManager::startAsyncScan() {
    if (_scanning) return;
    _scanning = true;
    WiFi.scanNetworks(true, true);   // async, show hidden
    Serial.println("[WiFi] Фоновий скан запущено");
}

void WiFiManager::checkAsyncScan() {
    if (!_scanning) return;
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) return;
    _scanning = false;
    _processScanResults(n);
    if (_scanCount > 0)
        Serial.printf("[WiFi] Фоновий скан: %d мереж\n", _scanCount);
}

// ---------------------------------------------------------------
String WiFiManager::ssid() const {
    if (_state == WiFiState::CAPTIVE_PORTAL) return String(AP_SSID)+" (AP)";
    if (_state == WiFiState::CONNECTED)      return WiFi.SSID();
    return "—";
}
String WiFiManager::ip() const {
    if (_state == WiFiState::CAPTIVE_PORTAL) return AP_IP.toString();
    if (_state == WiFiState::CONNECTED)      return WiFi.localIP().toString();
    return "—";
}
