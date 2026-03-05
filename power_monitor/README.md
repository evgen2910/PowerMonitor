# ESP32 Power Monitor v1.4.0

Моніторинг мережевого живлення та батареї з Telegram-сповіщеннями і веб-інтерфейсом.

## Підтримувані плати

| Плата | `#define` | Board в Arduino IDE |
|---|---|---|
| ESP32 WROOM/WROVER/DevKit | *(за замовч.)* | ESP32 Dev Module |
| **ESP32-C3 SuperMini** | `BOARD_ESP32C3` | ESP32C3 Dev Module |
| ESP32-C3 XIAO | `BOARD_ESP32C3` | XIAO_ESP32C3 |
| ESP32-S2 | `BOARD_ESP32S2` | ESP32-S2 Dev Module |
| ESP32-S3 | `BOARD_ESP32S3` | ESP32-S3 Dev Module |

## Налаштування для ESP32-C3 SuperMini

1. Відкрий `Config.h`, розкоментуй рядок:
   ```cpp
   #define BOARD_ESP32C3
   ```

2. В Arduino IDE:
   - Board: **ESP32C3 Dev Module**
   - USB CDC On Boot: **Enabled** (для Serial через USB)
   - Partition Scheme: Default 4MB with spiffs

3. GPIO на ESP32-C3 SuperMini:
   - Доступні цифрові: GPIO 0–10
   - ADC (для батареї): GPIO 0–4 (ADC1)
   - LED вбудований: GPIO 8 (active LOW)
   - GPIO 11–17: USB D-/D+ та flash — **не використовувати**

## Виправлення (v1.3.3 → v1.4.0)

- ✅ Мультиплатна підтримка (ESP32, C3, S2, S3)
- ✅ Правильні GPIO масиви для кожної плати
- ✅ Перевірка конфлікту пінів GPIO (живлення ≠ ADC)
- ✅ Збереження без перезавантаження (тільки WiFi/mDNS вимагає reboot)
- ✅ Окремі Chat ID для особистого чату та групи/каналу
- ✅ Авто-визначення Chat ID показує результат одразу
- ✅ Виправлено чекбокси в секції батареї (ADC)
- ✅ Коефіцієнт корекції ADC (batt_adc_coeff)
- ✅ ADC атенюація: per-pin для C3/S2/S3, глобальна для ESP32 classic

## Залежності

Тільки ESP32 Arduino Core (без сторонніх бібліотек).
