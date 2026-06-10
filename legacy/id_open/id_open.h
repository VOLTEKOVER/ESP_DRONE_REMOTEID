/* -*- tab-width: 2; mode: c; -*-
 *
 * C++ class for OpenDroneID on ESP-IDF
 * Converted from Arduino to ESP-IDF
 *
 * Copyright (c) 2020-2023, Steve Jack.
 *
 * MIT licence.
 *
 */

#ifndef ID_OPENDRONE_H
#define ID_OPENDRONE_H

#ifdef __cplusplus
extern "C" {
#endif

// ==================== ESP-IDF OpenDroneID Configuration ====================

#define ID_OD_WIFI_NAN    0
#define ID_OD_WIFI_BEACON 1
#define ID_OD_BT          0        // ASTM F3411-19 / ASD-STAN 4709-002.

#define USE_BEACON_FUNC   0
#define ESP32_WIFI_OPTION 0

#if ID_OD_WIFI_NAN || ID_OD_WIFI_BEACON
#define ID_OD_WIFI        1
#else
#define ID_OD_WIFI        0
#endif

#define ID_JAPAN          0        // Experimental

#if (ID_JAPAN) && (ID_OD_WIFI_NAN || USE_BEACON_FUNC || !ID_OD_WIFI_BEACON)
#warning "National IDs will only work with WIFI_BEACON"
#define ID_NATIONAL       0
#else
#define ID_NATIONAL       ID_JAPAN
#endif

#define WIFI_CHANNEL      6        // Be careful changing this.
#define BEACON_FRAME_SIZE 512
#define BEACON_INTERVAL   0        // ms, defaults to 500. Android apps would prefer 100ms.

// Used by the id_open_beacon and id_open_esp32.
 
#if ID_JAPAN
#define WIFI_COUNTRY_CC    "JP"
#define WIFI_COUNTRY_NCHAN 13
#else
#define WIFI_COUNTRY_CC    "ZZ"
#define WIFI_COUNTRY_NCHAN 11
#endif

#define ID_OD_AUTH_DATUM  1546300800LU

// ==================== Includes ====================

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "utm.h"
#include "opendroneid.h"

// ==================== Function Declarations ====================
//
// Functions in a processor specific file.
//

void     construct2(void);
void     init2(char *,int,uint8_t *,uint8_t);
uint8_t *capability(void);
int      tag_rates(uint8_t *,int);
int      tag_ext_rates(uint8_t *,int);
int      misc_tags(uint8_t *,int);
int      transmit_wifi2(uint8_t *,int);
int      transmit_ble2(uint8_t *,int);

// ==================== Utility Functions ====================

extern int      clock_gettime(clockid_t,struct timespec *);
extern uint64_t alt_unix_secs(int,int,int,int,int,int);

// Log output wrapper
static inline void id_open_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Time function wrapper
static inline uint32_t id_open_millis(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

// Delay function wrapper
static inline void id_open_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

#ifdef __cplusplus
}
#endif

#endif /* ID_OPENDRONE_H */
