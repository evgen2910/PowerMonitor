#include "Config.h"

// ── Версія схеми NVS — збільшуй лише коли треба скинути все ──
#define NVS_SCHEMA_VERSION  1

void ConfigManager::setDefaults() {
    strlcpy(cfg.wifi_ssid1,   "",              sizeof(cfg.wifi_ssid1));
    strlcpy(cfg.wifi_pass1,   "",              sizeof(cfg.wifi_pass1));
    strlcpy(cfg.wifi_ssid2,   "",              sizeof(cfg.wifi_ssid2));
    strlcpy(cfg.wifi_pass2,   "",              sizeof(cfg.wifi_pass2));
    strlcpy(cfg.bot_token,    "",              sizeof(cfg.bot_token));
    strlcpy(cfg.chat_id,      "",              sizeof(cfg.chat_id));
    strlcpy(cfg.group_chat_id,"",             sizeof(cfg.group_chat_id));
    cfg.chat_type          = 0;
    cfg.notify_power_on    = true;
    cfg.notify_power_off   = true;
    cfg.notify_boot        = true;
    cfg.notify_batt_low    = true;
    cfg.power_pin          = DEFAULT_POWER_PIN;
    cfg.power_pin_mode     = 0;
    cfg.power_active_low   = false;
    cfg.batt_enabled       = false;
    cfg.batt_pin           = DEFAULT_BATT_PIN;
    cfg.batt_r1_kohm       = 100.0f;
    cfg.batt_r2_kohm       = 27.0f;
    cfg.batt_cells         = 3;        // 3S за замовч. (відповідає R1=100k/R2=27k)
    cfg.batt_full_v        = 12.6f;    // 3S Li-ion full
    cfg.batt_empty_v       = 9.0f;     // 3S Li-ion empty
    strlcpy(cfg.ntp_server, "pool.ntp.org",  sizeof(cfg.ntp_server));
    cfg.tz_offset_sec      = 7200;
    strlcpy(cfg.device_name,"Power Monitor", sizeof(cfg.device_name));
    strlcpy(cfg.mdns_name,  "powermon",      sizeof(cfg.mdns_name));
#if defined(BOARD_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    cfg.board_type = 1;
#elif defined(BOARD_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S2)
    cfg.board_type = 2;
#elif defined(BOARD_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3)
    cfg.board_type = 3;
#else
    cfg.board_type = 0;
#endif
}

// ── Зберігаємо кожне поле окремим ключем ─────────────────────
void ConfigManager::save() {
    _prefs.begin("pm", false);
    _prefs.putUChar ("schema",    NVS_SCHEMA_VERSION);
    _prefs.putString("s1",        cfg.wifi_ssid1);
    _prefs.putString("p1",        cfg.wifi_pass1);
    _prefs.putString("s2",        cfg.wifi_ssid2);
    _prefs.putString("p2",        cfg.wifi_pass2);
    _prefs.putString("tok",       cfg.bot_token);
    _prefs.putString("cid",       cfg.chat_id);
    _prefs.putString("gcid",      cfg.group_chat_id);
    _prefs.putUChar ("ctype",     cfg.chat_type);
    _prefs.putBool  ("non",       cfg.notify_power_on);
    _prefs.putBool  ("noff",      cfg.notify_power_off);
    _prefs.putBool  ("nboot",     cfg.notify_boot);
    _prefs.putBool  ("nbatt",     cfg.notify_batt_low);
    _prefs.putUChar ("pin",       cfg.power_pin);
    _prefs.putUChar ("pmode",     cfg.power_pin_mode);
    _prefs.putBool  ("pinv",      cfg.power_active_low);
    _prefs.putBool  ("batten",    cfg.batt_enabled);
    _prefs.putUChar ("battpin",   cfg.batt_pin);
    _prefs.putFloat ("battr1",    cfg.batt_r1_kohm);
    _prefs.putFloat ("battr2",    cfg.batt_r2_kohm);
    _prefs.putFloat ("battfull",  cfg.batt_full_v);
    _prefs.putFloat ("battempty", cfg.batt_empty_v);
    _prefs.putUChar ("battcells", cfg.batt_cells);
    _prefs.putString("ntp",       cfg.ntp_server);
    _prefs.putInt   ("tz",        cfg.tz_offset_sec);
    _prefs.putString("dname",     cfg.device_name);
    _prefs.putString("mdns",      cfg.mdns_name);
    _prefs.putUChar ("brdtype",   cfg.board_type);
    _prefs.end();
    Serial.println("[CFG] Збережено (" + String(sizeof(cfg)) + " байт)");
}

// ── Завантажуємо поле-за-полем; відсутнє поле = дефолт ───────
// Це безпечно при будь-яких змінах структури між версіями OTA
void ConfigManager::load() {
    setDefaults();   // спочатку заповнюємо дефолтами

    _prefs.begin("pm", true);   // read-only

    // Якщо namespace порожній — перший запуск
    if (!_prefs.isKey("schema")) {
        _prefs.end();
        Serial.println("[CFG] Перший запуск — зберігаємо дефолти");
        save();
        return;
    }

    uint8_t schema = _prefs.getUChar("schema", 0);
    if (schema != NVS_SCHEMA_VERSION) {
        _prefs.end();
        Serial.printf("[CFG] Schema %d → %d, скидаємо\n", schema, NVS_SCHEMA_VERSION);
        reset();
        return;
    }

    // Зчитуємо кожне поле — якщо ключ відсутній (нова прошивка додала поле),
    // залишається дефолт із setDefaults() вище
#define GS(dst, key) { String v=_prefs.getString(key,""); if(v.length()) strlcpy(dst,v.c_str(),sizeof(dst)); }
    GS(cfg.wifi_ssid1,  "s1");
    GS(cfg.wifi_pass1,  "p1");
    GS(cfg.wifi_ssid2,  "s2");
    GS(cfg.wifi_pass2,  "p2");
    GS(cfg.bot_token,      "tok");
    GS(cfg.chat_id,        "cid");
    GS(cfg.group_chat_id,  "gcid");
    GS(cfg.ntp_server,  "ntp");
    GS(cfg.device_name, "dname");
    GS(cfg.mdns_name,   "mdns");
#undef GS

    cfg.chat_type       = _prefs.getUChar ("ctype",     cfg.chat_type);
    cfg.notify_power_on = _prefs.getBool  ("non",       cfg.notify_power_on);
    cfg.notify_power_off= _prefs.getBool  ("noff",      cfg.notify_power_off);
    cfg.notify_boot     = _prefs.getBool  ("nboot",     cfg.notify_boot);
    cfg.notify_batt_low = _prefs.getBool  ("nbatt",     cfg.notify_batt_low);
    cfg.power_pin       = _prefs.getUChar ("pin",       cfg.power_pin);
    cfg.power_pin_mode  = _prefs.getUChar ("pmode",     cfg.power_pin_mode);
    cfg.power_active_low= _prefs.getBool  ("pinv",      cfg.power_active_low);
    cfg.batt_enabled    = _prefs.getBool  ("batten",    cfg.batt_enabled);
    cfg.batt_pin        = _prefs.getUChar ("battpin",   cfg.batt_pin);
    cfg.batt_r1_kohm    = _prefs.getFloat ("battr1",    cfg.batt_r1_kohm);
    cfg.batt_r2_kohm    = _prefs.getFloat ("battr2",    cfg.batt_r2_kohm);
    cfg.batt_full_v     = _prefs.getFloat ("battfull",  cfg.batt_full_v);
    cfg.batt_empty_v    = _prefs.getFloat ("battempty", cfg.batt_empty_v);
    cfg.batt_cells      = _prefs.getUChar ("battcells", cfg.batt_cells);
    cfg.tz_offset_sec   = _prefs.getInt   ("tz",        cfg.tz_offset_sec);
    cfg.board_type      = _prefs.getUChar ("brdtype",   cfg.board_type);

    _prefs.end();
    Serial.println("[CFG] Завантажено з NVS");
}

void ConfigManager::reset() {
    _prefs.begin("pm", false);
    _prefs.clear();
    _prefs.end();
    setDefaults();
    Serial.println("[CFG] Factory reset");
    save();
}
