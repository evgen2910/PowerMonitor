#pragma once
#include <Arduino.h>
#include <WebServer.h>

class WebUI {
public:
    static WebUI& instance() { static WebUI w; return w; }
    void begin();
    void loop();
private:
    WebUI() : _srv(80) {}
    WebServer _srv;

    String _head(const String& title) const;
    String _nav(const String& active) const;
    static String _foot();

    void _handleRoot();
    void _handleConfig();
    void _handleSave();
    void _handleStatus();
    void _handleScan();           // NEW: /api/scan
    void _handleLogs();
    void _handleGetChatId();
    void _handleSendTest();
    void _handleOtaPage();
    void _handleOtaUrl();
    void _handleOtaUpload();
    void _handleOtaUploadDone();
    void _handleReboot();
    void _handleFactoryReset();
    void _handleCaptive();

    static String _esc(const String& s);
    static String _checked(bool v);
    static String _sel(uint8_t cur, uint8_t val, const char* label);
    void _redirect(const String& url, int code = 303);
};

inline WebUI& UI() { return WebUI::instance(); }
