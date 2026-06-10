#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_remote_id.h"
#include "protocol_detect.h"
#include "nmea_parser.h"
#include "msp_parser.h"
#include "mavlink_parser.h"
#include "wifi_tx.h"
#include "ble_tx.h"
#include "web_config.h"
#include "nvs_storage.h"

#define TAG "ESP_RID"

static rid_config_t g_config;
static rid_state_t g_state;
static bool g_running = false;

static void default_config(rid_config_t *cfg)
{
    memset(cfg, 0, sizeof(rid_config_t));

    cfg->protocol = RID_PROTOCOL_AUTO;
    cfg->uart_port = 1;
    cfg->baud_rate = 57600;
    cfg->tx_pin = 17;
    cfg->rx_pin = 18;

    cfg->ua_type = 1;
    cfg->id_type = 1;
    strcpy(cfg->uas_id, "ESP32-RID-001");
    strcpy(cfg->operator_id, "OP-UNKNOWN");

    cfg->ua_type_2 = 0;
    cfg->id_type_2 = 0;
    strcpy(cfg->uas_id_2, "");

    cfg->tx_modes = RID_TRANSMIT_WIFI_BCN;
    cfg->wifi_channel = 6;
    cfg->wifi_power_dbm = 20.0f;
    cfg->wifi_bcn_rate_hz = 1.0f;
    cfg->wifi_nan_rate_hz = 0.0f;
    cfg->ble4_rate_hz = 1.0f;
    cfg->ble4_power_dbm = 18.0f;
    cfg->ble5_rate_hz = 1.0f;
    cfg->ble5_power_dbm = 18.0f;

    strcpy(cfg->wifi_ssid, "ESP-RID");
    strcpy(cfg->wifi_password, "");
    cfg->webserver_en = 1;

    cfg->mavlink_sysid = 0;
    cfg->bcast_powerup = 1;

    cfg->options = 0;
    cfg->lock_level = 0;

    for (int i = 0; i < ESP_RID_NUM_KEYS; i++)
        strcpy(cfg->public_keys[i], "");
}

void esp_rid_init(void)
{
    ESP_LOGI(TAG, "ESP Remote ID v%s initializing", ESP_RID_VERSION);

    default_config(&g_config);
    memset(&g_state, 0, sizeof(rid_state_t));

    nvs_storage_init();
    nvs_storage_load(&g_config);

    protocol_detect_init();
    nmea_parser_init();
    msp_parser_init();
    mavlink_parser_init();

    wifi_tx_init();
    ble_tx_init();

    web_config_init();

    ESP_LOGI(TAG, "ESP Remote ID initialized");
}

void esp_rid_set_config(const rid_config_t *config)
{
    memcpy(&g_config, config, sizeof(rid_config_t));
    nvs_storage_save(&g_config);
}

void esp_rid_get_config(rid_config_t *config)
{
    memcpy(config, &g_config, sizeof(rid_config_t));
}

void esp_rid_get_state(rid_state_t *state)
{
    memcpy(state, &g_state, sizeof(rid_state_t));
}

void esp_rid_factory_reset(void)
{
    nvs_storage_erase();
    default_config(&g_config);
    nvs_storage_save(&g_config);
}

static void update_transmissions(void)
{
    if (!g_state.gps_valid && !g_config.bcast_powerup) return;

    if (g_state.active_protocol == RID_PROTOCOL_UNKNOWN) return;

    if (g_config.tx_modes & RID_TRANSMIT_WIFI_BCN) {
        wifi_tx_transmit(&g_state.gps, &g_state.identity);
        g_state.wifi_bcn_count++;
        g_state.transmissions_count++;
    }

    if (g_config.tx_modes & RID_TRANSMIT_BLE4) {
        ble_tx_transmit_legacy(&g_state.gps, &g_state.identity);
        g_state.ble4_count++;
        g_state.transmissions_count++;
    }

    if (g_config.tx_modes & RID_TRANSMIT_BLE5) {
        ble_tx_transmit_lr(&g_state.gps, &g_state.identity);
        g_state.ble5_count++;
        g_state.transmissions_count++;
    }
}

static void rid_task(void *arg)
{
    ESP_LOGI(TAG, "ESP Remote ID task started");
    g_state.active_protocol = RID_PROTOCOL_UNKNOWN;

    while (g_running) {
        rid_gps_data_t gps_data;
        rid_protocol_t proto = RID_PROTOCOL_UNKNOWN;

        memset(&gps_data, 0, sizeof(rid_gps_data_t));

        if (g_config.protocol == RID_PROTOCOL_AUTO)
            proto = protocol_detect_auto();
        else
            proto = g_config.protocol;

        g_state.active_protocol = proto;

        switch (proto) {
        case RID_PROTOCOL_NMEA:
            nmea_parser_get(&gps_data);
            break;
        case RID_PROTOCOL_MSP:
            msp_parser_get(&gps_data);
            break;
        case RID_PROTOCOL_MAVLINK:
            mavlink_parser_get(&gps_data);
            break;
        default:
            break;
        }

        if (gps_data.fix_type >= 2 && gps_data.latitude != 0.0) {
            memcpy(&g_state.gps, &gps_data, sizeof(rid_gps_data_t));
            g_state.gps_valid = true;
            g_state.last_update_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            strncpy(g_state.identity.uas_id, g_config.uas_id, ESP_RID_MAX_STR_LEN);
            strncpy(g_state.identity.operator_id, g_config.operator_id, ESP_RID_MAX_STR_LEN);
            g_state.identity.id_type = g_config.id_type;
            g_state.identity.ua_type = g_config.ua_type;

            update_transmissions();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

void esp_rid_start(void)
{
    if (g_running) return;
    g_running = true;
    xTaskCreate(rid_task, "rid_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "ESP Remote ID started");
}

void esp_rid_stop(void)
{
    g_running = false;
    ESP_LOGI(TAG, "ESP Remote ID stopped");
}
