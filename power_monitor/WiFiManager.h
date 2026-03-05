#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "Config.h"

enum class WiFiState { DISCONNECTED, CONNECTING, CONNECTED, CAPTIVE_PORTAL };

struct ScannedNet {
    String  ssid;
    int32_t rssi;
    uint8_t encType;    // WIFI_AUTH_OPEN etc.
    bool    saved;      // чи збережена в cfg
};

class WiFiManager {
public:
    static WiFiManager& instance() { static WiFiManager w; return w; }

    void      begin();
    void      loop();

    bool      isConnected() const { return _state == WiFiState::CONNECTED; }
    bool      isCaptive()   const { return _state == WiFiState::CAPTIVE_PORTAL; }
    WiFiState state()       const { return _state; }

    String    ssid()  const;
    String    ip()    const;
    int8_t    rssi()  const { return WiFi.RSSI(); }

    void      startCaptivePortal();
    void      reconnectNow();
    void      restartMDNS();

    // Сканування мереж (синхронне, ~3с)
    int       scanNetworks();           // повертає кількість знайдених
    int       scannedCount()  const { return _scanCount; }
    const ScannedNet& scannedNet(int i) const { return _nets[i]; }
    bool      scanInProgress() const { return _scanning; }
    void      startAsyncScan();         // запускає фоновий скан
    void      checkAsyncScan();         // перевірити результат (у loop)

    DNSServer dns;

private:
    WiFiManager() {}
    bool  _tryConnect(const char* ssid, const char* pass);
    void  _startSTA();
    void  _startMDNS();
    void  _processScanResults(int n);

    WiFiState  _state        = WiFiState::DISCONNECTED;
    uint32_t   _lastAttempt  = 0;
    uint8_t    _retryCount   = 0;   // лічильник невдалих спроб підключення

    ScannedNet _nets[WIFI_SCAN_MAX];
    int        _scanCount   = 0;
    bool       _scanning    = false;
};

inline WiFiManager& WM() { return WiFiManager::instance(); }
