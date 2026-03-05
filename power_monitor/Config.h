#pragma once
#include <Arduino.h>
#include <Preferences.h>

// ============================================================
//  Константи прошивки
// ============================================================
#define FW_VERSION          "5.3"
#define AP_SSID             "PowerMon-Setup"
#define AP_PASS             "12345678"
#define AP_IP               IPAddress(192,168,4,1)
#define LED_PIN             LED_BUILTIN

// Таймінги
#define WIFI_CONNECT_TIMEOUT_MS    15000UL
#define WIFI_RECONNECT_INTERVAL_MS 180000UL   // 3 хв між спробами
#define DEBOUNCE_MS                3000UL
#define MSG_QUEUE_PROCESS_MS       2500UL
#define NTP_SYNC_INTERVAL_MS       3600000UL
#define STATUS_BLINK_CONNECTED_MS  2000UL
#define STATUS_BLINK_PORTAL_MS     150UL

// Черга Telegram
#define MSG_QUEUE_SIZE      20
#define MSG_MAX_RETRIES     5
#define MSG_RETRY_DELAY_MS  4000UL

// Журнал
#define EVENT_LOG_SIZE      60

// Telegram
#define TG_HOST             "api.telegram.org"
#define TG_PORT             443

// ============================================================
//  Вибір плати — розкоментуй потрібну або визнач через build flags
//  -D BOARD_ESP32C3    → ESP32-C3 SuperMini / XIAO C3 / DevKitM-1
//  -D BOARD_ESP32S2    → ESP32-S2
//  -D BOARD_ESP32S3    → ESP32-S3
//  За замовчуванням    → ESP32 classic (WROOM / WROVER / DevKit)
// ============================================================
// #define BOARD_ESP32C3
// #define BOARD_ESP32S2
// #define BOARD_ESP32S3

// ── ESP32-C3 (RISC-V) — SuperMini / XIAO / DevKitM ──────────
#if defined(BOARD_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C3)
  #define BOARD_NAME          "ESP32-C3"
  #undef  LED_PIN
  #define LED_PIN             8           // SuperMini: GPIO8 = вбудований LED (active LOW)
  #define LED_ACTIVE_LOW      1           // LED вмикається LOW
  // GPIO придатні для цифрового вводу (0-10; 11-17 = USB/flash на деяких модулях)
  static const uint8_t GPIO_INPUT_PINS[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  static const uint8_t GPIO_INPUT_COUNT  = sizeof(GPIO_INPUT_PINS);
  // Input-only пінів немає на C3 — всі підтримують OUTPUT теж
  static const uint8_t GPIO_INPUT_ONLY[] = {};
  static const uint8_t GPIO_INPUT_ONLY_COUNT = 0;
  // ADC1: GPIO 0-4 (безпечні з WiFi на C3)
  static const uint8_t GPIO_ADC_PINS[]   = { 0, 1, 2, 3, 4 };
  static const uint8_t GPIO_ADC_PINS_COUNT = sizeof(GPIO_ADC_PINS);
  // Дефолтні піни для C3 SuperMini
  #define DEFAULT_POWER_PIN   3
  #define DEFAULT_BATT_PIN    4
  #define HAS_ADC_ATTENUATION_PER_PIN 1   // C3: треба analogSetPinAttenuation()

// ── ESP32-S2 ─────────────────────────────────────────────────
#elif defined(BOARD_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S2)
  #define BOARD_NAME          "ESP32-S2"
  #undef  LED_PIN
  #define LED_PIN             15
  #define LED_ACTIVE_LOW      0
  static const uint8_t GPIO_INPUT_PINS[] = {
      1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
      21, 33, 34, 35, 36, 37, 38
  };
  static const uint8_t GPIO_INPUT_COUNT  = sizeof(GPIO_INPUT_PINS);
  static const uint8_t GPIO_INPUT_ONLY[] = {};
  static const uint8_t GPIO_INPUT_ONLY_COUNT = 0;
  // ADC1: GPIO 1-10
  static const uint8_t GPIO_ADC_PINS[]   = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  static const uint8_t GPIO_ADC_PINS_COUNT = sizeof(GPIO_ADC_PINS);
  #define DEFAULT_POWER_PIN   5
  #define DEFAULT_BATT_PIN    6
  #define HAS_ADC_ATTENUATION_PER_PIN 1

// ── ESP32-S3 ─────────────────────────────────────────────────
#elif defined(BOARD_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3)
  #define BOARD_NAME          "ESP32-S3"
  #undef  LED_PIN
  #define LED_PIN             48
  #define LED_ACTIVE_LOW      0
  static const uint8_t GPIO_INPUT_PINS[] = {
      1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
      21, 38, 39, 40, 41, 42, 45, 46, 47, 48
  };
  static const uint8_t GPIO_INPUT_COUNT  = sizeof(GPIO_INPUT_PINS);
  static const uint8_t GPIO_INPUT_ONLY[] = {};
  static const uint8_t GPIO_INPUT_ONLY_COUNT = 0;
  // ADC1: GPIO 1-10
  static const uint8_t GPIO_ADC_PINS[]   = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  static const uint8_t GPIO_ADC_PINS_COUNT = sizeof(GPIO_ADC_PINS);
  #define DEFAULT_POWER_PIN   5
  #define DEFAULT_BATT_PIN    6
  #define HAS_ADC_ATTENUATION_PER_PIN 1

// ── ESP32 classic (WROOM / WROVER / DevKit) ──────────────────
#else
  #define BOARD_NAME          "ESP32"
  #define LED_ACTIVE_LOW      0
  // Виключаємо: 6-11 (SPI flash), 20, 24, 28-31 (зарезервовані)
  static const uint8_t GPIO_INPUT_PINS[] = {
      0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19,
      21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39
  };
  static const uint8_t GPIO_INPUT_COUNT  = sizeof(GPIO_INPUT_PINS);
  // Input-only (без OUTPUT, без підтяжок): 34, 35, 36(VP), 39(VN)
  static const uint8_t GPIO_INPUT_ONLY[] = { 34, 35, 36, 39 };
  static const uint8_t GPIO_INPUT_ONLY_COUNT = sizeof(GPIO_INPUT_ONLY);
  // ADC1 (не конфліктує з WiFi): GPIO 32-39
  static const uint8_t GPIO_ADC_PINS[]   = { 32, 33, 34, 35, 36, 39 };
  static const uint8_t GPIO_ADC_PINS_COUNT = sizeof(GPIO_ADC_PINS);
  #define DEFAULT_POWER_PIN   34
  #define DEFAULT_BATT_PIN    36
  #define HAS_ADC_ATTENUATION_PER_PIN 0   // ESP32 classic: analogSetAttenuation() глобально
#endif

// WiFi scan
#define WIFI_SCAN_MAX       10      // макс. мереж у списку

// Battery ADC
#define BATT_ADC_SAMPLES    16      // середнє по N вимірах (згладжування)
// analogReadMilliVolts() використовує вбудовану калібровку — VREF/RESOLUTION не потрібні
#define BATT_UPDATE_MS      10000UL // інтервал оновлення напруги
#define BATT_LOW_THRESHOLD  20      // % → сповіщення "батарея розряджена"

// ============================================================
//  Структури
// ============================================================
struct TgMessage {
    String   text;
    uint32_t addedAt;
    uint8_t  retries;
    bool     delivered;
};

struct EventEntry {
    String   text;
    uint32_t ts;
};

struct Config {
    // WiFi
    char    wifi_ssid1[64];
    char    wifi_pass1[64];
    char    wifi_ssid2[64];
    char    wifi_pass2[64];

    // Telegram
    char    bot_token[128];
    char    chat_id[32];        // Chat ID для особистого чату
    char    group_chat_id[32];  // Chat ID для групи/каналу
    uint8_t chat_type;          // 0=особистий, 1=група/канал

    // Сповіщення
    bool    notify_power_on;
    bool    notify_power_off;
    bool    notify_boot;
    bool    notify_batt_low;    // сповіщення "батарея низька"

    // GPIO — живлення мережі
    uint8_t power_pin;
    uint8_t power_pin_mode;     // 0=INPUT, 1=PULLUP, 2=PULLDOWN
    bool    power_active_low;

    // GPIO — батарея (ADC)
    bool    batt_enabled;       // моніторинг батареї увімкнено
    uint8_t batt_pin;           // ADC пін (ADC1)
    float   batt_r1_kohm;       // R1 дільника (до Vbatt), кОм
    float   batt_r2_kohm;       // R2 дільника (до GND), кОм
    uint8_t batt_cells;         // кількість cells: 1S/2S/3S/4S (визначає full/empty автоматично)
    float   batt_full_v;        // напруга "повна" батарея, В
    float   batt_empty_v;       // напруга "порожня" батарея, В

    // Час
    char    ntp_server[64];
    int32_t tz_offset_sec;

    // Пристрій
    char    device_name[64];
    char    mdns_name[32];
    uint8_t board_type;         // 0=ESP32, 1=ESP32-C3, 2=ESP32-S2, 3=ESP32-S3
};

// ============================================================
//  ConfigManager singleton
// ============================================================
class ConfigManager {
public:
    static ConfigManager& instance() {
        static ConfigManager inst;
        return inst;
    }
    Config cfg;
    void load();
    void save();
    void reset();
private:
    ConfigManager() {}
    void setDefaults();
    Preferences _prefs;
};

inline ConfigManager& Cfg() { return ConfigManager::instance(); }
