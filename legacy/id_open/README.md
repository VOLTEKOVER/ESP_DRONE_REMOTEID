# id_open Component - ESP-IDF OpenDroneID

## Conversione da Arduino a ESP-IDF

Questo componente è stata la conversione della libreria `id_open` da Arduino a ESP-IDF puro.

### Cambiamenti principali

- **Rimosso**: `#include <Arduino.h>`
- **Convertito**: `Serial.print()` → `printf()` / `esp_log`
- **Convertito**: `millis()` → `xTaskGetTickCount() * portTICK_PERIOD_MS`
- **Convertito**: `delay()` → `vTaskDelay()`
- **Convertito**: `WiFi.h` → `esp_wifi.h`
- **Convertito**: `BLEDevice.h` → `esp_bt_defs.h` (BLE API di ESP-IDF)

### File componente

- `CMakeLists.txt` - Configurazione build ESP-IDF
- `id_open.h` - Header principale (ESP-IDF)
- `id_open.cpp` - Implementazione principale (convertito)
- `id_open_esp32.cpp` - Code specifico ESP32 (convertito)
- `id_open_beacon.cpp` - Beacon WiFi (convertito)
- `alt_unix_time.c` - Calcolo timestamp Unix

### Dipendenze

- `esp_wifi` - WiFi framework
- `esp_event` - Event loop
- `nvs_flash` - Non-volatile storage
- `esp_system` - System utilities
- `freertos` - Real-time OS
- `bt` - Bluetooth (opzionale per BLE)

### Configurazione

Tutti i parametri di configurazione (WiFi NAN, beacon, BLE) sono definiti in `id_open.h`.

Modifica le macro per adattare al tuo caso d'uso:
```c
#define ID_OD_WIFI_NAN    0  // Cambio NAN WiFi
#define ID_OD_WIFI_BEACON 1  // Beacon WiFi
#define ID_OD_BT          0  // Bluetooth
```

### Note

- Component richiede ESP-IDF 4.4 o superiore
- Tutti i riferimenti Arduino sono stati rimossi
- Completamente nativa ESP-IDF
- Thread-safe con FreeRTOS
