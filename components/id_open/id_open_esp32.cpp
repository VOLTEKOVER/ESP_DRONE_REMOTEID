/* -*- tab-width: 2; mode: c; -*-
 * 
 * C++ class for OpenDroneID on ESP-IDF
 * This file has the ESP32 specific code.
 *
 * Copyright (c) 2020-2023, Steve Jack.
 *
 * MIT licence.
 *
 */

#pragma GCC diagnostic warning "-Wunused-variable"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "id_open.h"

#define TAG "ID_OPEN_ESP32"

// WiFi ESP32 specific implementation

void construct2(void) {
    ESP_LOGI(TAG, "construct2() - ESP32 WiFi initialization");
}

void init2(char *ssid, int ssid_length, uint8_t *WiFi_mac_addr, uint8_t wifi_channel) {
    esp_err_t ret;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    ESP_LOGI(TAG, "init2() - Initializing WiFi for SSID: %.*s", ssid_length, ssid);
    
    // Initialize WiFi
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Set storage to RAM only
    ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_storage failed: %s", esp_err_to_name(ret));
    }
    
    // Set AP mode
    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
    }
    
    // Configure AP
    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = (uint8_t)ssid_length,
            .channel = wifi_channel,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 4,
            .beacon_interval = 1000,
        },
    };
    
    strncpy((char *)ap_config.ap.ssid, ssid, ssid_length);
    
    ret = esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(ret));
    }
    
    // Start WiFi
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
    }
    
    // Set country code
    wifi_country_t country = {WIFI_COUNTRY_CC, 1, 11, 20, WIFI_COUNTRY_POLICY_AUTO};
    ret = esp_wifi_set_country(&country);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_set_country failed: %s", esp_err_to_name(ret));
    }
    
    // Get MAC address
    ret = esp_read_mac(WiFi_mac_addr, ESP_MAC_WIFI_STA);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WiFi MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                 WiFi_mac_addr[0], WiFi_mac_addr[1], WiFi_mac_addr[2],
                 WiFi_mac_addr[3], WiFi_mac_addr[4], WiFi_mac_addr[5]);
    }
}

uint8_t *capability(void) {
    static uint8_t cap[] = {
        0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    return cap;
}

int tag_rates(uint8_t *buf, int len) {
    if (len < 2) return 0;
    
    buf[0] = 0x01;  // Supported Rates tag
    buf[1] = 0x08;  // Length
    buf[2] = 0x8c;  // 6 Mbps (basic)
    buf[3] = 0x12;  // 9 Mbps (basic)
    buf[4] = 0x98;  // 12 Mbps (basic)
    buf[5] = 0x24;  // 18 Mbps (basic)
    buf[6] = 0x30;  // 24 Mbps
    buf[7] = 0x48;  // 36 Mbps
    buf[8] = 0x60;  // 48 Mbps
    buf[9] = 0x6c;  // 54 Mbps
    
    return 10;
}

int tag_ext_rates(uint8_t *buf, int len) {
    return 0;
}

int misc_tags(uint8_t *buf, int len) {
    return 0;
}

int transmit_wifi2(uint8_t *buf, int len) {
    ESP_LOGD(TAG, "transmit_wifi2() - Transmitting %d bytes via WiFi", len);
    
    // Use esp_wifi_80211_tx for raw frame transmission
    // This function requires the frame to be complete with MAC header
    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_AP, buf, len, true);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_80211_tx failed: %s", esp_err_to_name(ret));
        return -1;
    }
    
    return 0;
}

int transmit_ble2(uint8_t *buf, int len) {
    ESP_LOGD(TAG, "transmit_ble2() - BLE transmission not yet implemented");
    return 0;
}
