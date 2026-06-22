#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "linenoise/linenoise.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cli.h"
#include "esp_remote_id.h"
#include "protocol_detect.h"

#define TAG "CLI"
#define CLI_STACK_SIZE 4096

static const char *proto_name(rid_protocol_t p)
{
    switch (p) {
        case RID_PROTOCOL_UNKNOWN: return "UNKNOWN";
        case RID_PROTOCOL_MAVLINK: return "MAVLink";
        case RID_PROTOCOL_MSP:     return "MSP";
        case RID_PROTOCOL_NMEA:   return "NMEA";
        case RID_PROTOCOL_NONE:   return "NONE";
        case RID_PROTOCOL_AUTO:   return "AUTO";
        default: return "?";
    }
}

static int cmd_status(int argc, char **argv)
{
    rid_config_t cfg;
    rid_state_t state;
    esp_rid_get_config(&cfg);
    esp_rid_get_state(&state);

    printf("\n  ESP DRONE REMOTEID v%s\n\n", ESP_RID_VERSION);
    printf("  Protocol  : %s\n", proto_name(state.active_protocol));
    printf("  GPS Fix   : %s  (fix_type=%d, sats=%u)\n",
           state.gps_valid ? "YES" : "NO", state.gps.fix_type, state.gps.satellites);
    printf("  Position  : %.6f / %.6f  alt=%.1f m\n",
           state.gps.latitude, state.gps.longitude, (double)state.gps.altitude_msl);
    printf("  Speed     : %.1f m/s  heading=%d\n", (double)state.gps.speed, state.gps.heading);
    printf("  TX total  : %lu  (BCN=%lu  NAN=%lu  BLE4=%lu  BLE5=%lu)\n",
           (unsigned long)state.transmissions_count,
           (unsigned long)state.wifi_bcn_count, (unsigned long)state.wifi_nan_count,
           (unsigned long)state.ble4_count, (unsigned long)state.ble5_count);
    uint32_t heap_free = esp_get_free_heap_size();
    uint32_t heap_total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    int64_t us = esp_timer_get_time();
    uint32_t sec = (uint32_t)(us / 1000000);
    printf("  Heap      : %lu KB / %lu KB\n", (unsigned long)(heap_free / 1024), (unsigned long)(heap_total / 1024));
    printf("  Uptime    : %02u:%02u:%02u\n", sec / 3600, (sec % 3600) / 60, sec % 60);
    printf("  Lock lvl  : %d\n\n", cfg.lock_level);
    return 0;
}

static int cmd_config(int argc, char **argv)
{
    rid_config_t cfg;
    esp_rid_get_config(&cfg);
    printf("\n  Configuration:\n\n");
    printf("  Protocol    : %s\n", proto_name(cfg.protocol));
    printf("  UART        : port=%d baud=%lu tx=%d rx=%d\n", cfg.uart_port, (unsigned long)cfg.baud_rate, cfg.tx_pin, cfg.rx_pin);
    printf("  UAS ID      : %s\n", cfg.uas_id);
    printf("  Operator ID : %s\n", cfg.operator_id);
    printf("  UA Type     : %d  ID Type: %d\n", cfg.ua_type, cfg.id_type);
    printf("  TX Modes    : ");
    if (cfg.tx_modes & RID_TRANSMIT_WIFI_BCN) printf("WiFi_BCN ");
    if (cfg.tx_modes & RID_TRANSMIT_WIFI_NAN) printf("WiFi_NAN ");
    if (cfg.tx_modes & RID_TRANSMIT_BLE4)     printf("BLE4 ");
    if (cfg.tx_modes & RID_TRANSMIT_BLE5)     printf("BLE5 ");
    printf("\n");
    printf("  WiFi        : CH=%d  power=%.1f dBm  SSID=%s\n", cfg.wifi_channel, (double)cfg.wifi_power_dbm, cfg.wifi_ssid);
    printf("  WiFi Beacon : %.1f Hz  NAN=%.1f Hz\n", (double)cfg.wifi_bcn_rate_hz, (double)cfg.wifi_nan_rate_hz);
    printf("  BLE4        : %.1f Hz  power=%.1f dBm\n", (double)cfg.ble4_rate_hz, (double)cfg.ble4_power_dbm);
    printf("  BLE5        : %.1f Hz  power=%.1f dBm\n", (double)cfg.ble5_rate_hz, (double)cfg.ble5_power_dbm);
    printf("  Operator    : %.6f / %.6f  alt=%.1f\n", cfg.operator_lat, cfg.operator_lon, (double)cfg.operator_alt);
    printf("  Options     : 0x%02x\n", cfg.options);
    printf("  Web server  : %s\n", cfg.webserver_en ? "enabled" : "disabled");
    printf("  Lock level  : %d\n\n", cfg.lock_level);
    return 0;
}

static int cmd_restart(int argc, char **argv)
{
    printf("Restarting...\n");
    esp_restart();
    return 0;
}

static int cmd_reset(int argc, char **argv)
{
    printf("Factory reset...\n");
    esp_rid_factory_reset();
    esp_restart();
    return 0;
}

static int cmd_protocol(int argc, char **argv)
{
    rid_config_t cfg;
    esp_rid_get_config(&cfg);
    if (argc == 1) {
        printf("Protocol: %s\n", proto_name(cfg.protocol));
        return 0;
    }
    rid_protocol_t p = RID_PROTOCOL_AUTO;
    if (strcasecmp(argv[1], "auto") == 0) p = RID_PROTOCOL_AUTO;
    else if (strcasecmp(argv[1], "mavlink") == 0) p = RID_PROTOCOL_MAVLINK;
    else if (strcasecmp(argv[1], "msp") == 0) p = RID_PROTOCOL_MSP;
    else if (strcasecmp(argv[1], "nmea") == 0) p = RID_PROTOCOL_NMEA;
    else if (strcasecmp(argv[1], "none") == 0) p = RID_PROTOCOL_NONE;
    else { printf("Unknown protocol: %s  (use: auto, mavlink, msp, nmea, none)\n", argv[1]); return 1; }
    cfg.protocol = p;
    esp_rid_set_config(&cfg);
    printf("Protocol set to: %s\n", proto_name(p));
    return 0;
}

static int cmd_heap(int argc, char **argv)
{
    uint32_t free = esp_get_free_heap_size();
    uint32_t total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    uint32_t min = esp_get_minimum_free_heap_size();
    printf("  Heap free : %lu KB / %lu KB\n", (unsigned long)(free / 1024), (unsigned long)(total / 1024));
    printf("  Heap min  : %lu KB\n", (unsigned long)(min / 1024));
    return 0;
}

static int cmd_log_level(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: log_level <tag> <NONE|ERROR|WARN|INFO|DEBUG|VERBOSE>\n");
        return 1;
    }
    esp_log_level_t level;
    if (strcasecmp(argv[2], "NONE") == 0) level = ESP_LOG_NONE;
    else if (strcasecmp(argv[2], "ERROR") == 0) level = ESP_LOG_ERROR;
    else if (strcasecmp(argv[2], "WARN") == 0) level = ESP_LOG_WARN;
    else if (strcasecmp(argv[2], "INFO") == 0) level = ESP_LOG_INFO;
    else if (strcasecmp(argv[2], "DEBUG") == 0) level = ESP_LOG_DEBUG;
    else if (strcasecmp(argv[2], "VERBOSE") == 0) level = ESP_LOG_VERBOSE;
    else { printf("Unknown level: %s\n", argv[2]); return 1; }
    esp_log_level_set(argv[1], level);
    printf("Log level for '%s' set to %s\n", argv[1], argv[2]);
    return 0;
}

static int cmd_patrol(int argc, char **argv)
{
    rid_config_t cfg;
    esp_rid_get_config(&cfg);
    if (argc >= 2 && strcasecmp(argv[1], "off") == 0)
        cfg.options &= ~RID_OPT_DEMO_MODE;
    else if (argc >= 2 && strcasecmp(argv[1], "on") == 0)
        cfg.options |= RID_OPT_DEMO_MODE;
    else
        cfg.options ^= RID_OPT_DEMO_MODE;
    esp_rid_set_config(&cfg);
    printf("Demo patrol mode: %s\n", (cfg.options & RID_OPT_DEMO_MODE) ? "ON" : "OFF");
    return 0;
}

static int cmd_transmit(int argc, char **argv)
{
    rid_config_t cfg;
    esp_rid_get_config(&cfg);
    if (argc < 2) {
        printf("  TX modes: %s%s%s%s\n",
            (cfg.tx_modes & RID_TRANSMIT_WIFI_BCN) ? "WiFi_BCN " : "",
            (cfg.tx_modes & RID_TRANSMIT_WIFI_NAN) ? "WiFi_NAN " : "",
            (cfg.tx_modes & RID_TRANSMIT_BLE4) ? "BLE4 " : "",
            (cfg.tx_modes & RID_TRANSMIT_BLE5) ? "BLE5 " : "");
        printf("  Usage: transmit <wifi_bcn|wifi_nan|ble4|ble5|all> <on|off>\n");
        return 0;
    }
    if (argc < 3) { printf("Missing on/off argument\n"); return 1; }
    uint8_t mask = 0;
    if (strcasecmp(argv[1], "wifi_bcn") == 0) mask = RID_TRANSMIT_WIFI_BCN;
    else if (strcasecmp(argv[1], "wifi_nan") == 0) mask = RID_TRANSMIT_WIFI_NAN;
    else if (strcasecmp(argv[1], "ble4") == 0) mask = RID_TRANSMIT_BLE4;
    else if (strcasecmp(argv[1], "ble5") == 0) mask = RID_TRANSMIT_BLE5;
    else if (strcasecmp(argv[1], "all") == 0) mask = 0x0F;
    else { printf("Unknown mode: %s\n", argv[1]); return 1; }
    bool on = (strcasecmp(argv[2], "on") == 0);
    if (mask == 0x0F) {
        cfg.tx_modes = on ? 0x0F : 0;
    } else if (on) {
        cfg.tx_modes |= mask;
    } else {
        cfg.tx_modes &= ~mask;
    }
    esp_rid_set_config(&cfg);
    printf("TX mode '%s' set to %s\n", argv[1], on ? "ON" : "OFF");
    return 0;
}

static int cmd_mac(int argc, char **argv)
{
    uint8_t mac_ap[6] = {0}, mac_sta[6] = {0};
    esp_wifi_get_mac(WIFI_IF_AP, mac_ap);
    esp_wifi_get_mac(WIFI_IF_STA, mac_sta);
    printf("  MAC AP  : %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac_ap[0], mac_ap[1], mac_ap[2], mac_ap[3], mac_ap[4], mac_ap[5]);
    printf("  MAC STA : %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac_sta[0], mac_sta[1], mac_sta[2], mac_sta[3], mac_sta[4], mac_sta[5]);
    return 0;
}

static int cmd_uptime(int argc, char **argv)
{
    int64_t us = esp_timer_get_time();
    uint32_t sec = (uint32_t)(us / 1000000);
    printf("  Uptime: %02u:%02u:%02u\n", sec / 3600, (sec % 3600) / 60, sec % 60);
    return 0;
}

static void register_commands(void)
{
    const esp_console_cmd_t cmds[] = {
        { .command = "status",   .help = "Show system status (protocol, GPS, TX, heap, uptime)", .func = cmd_status },
        { .command = "config",   .help = "Show current configuration", .func = cmd_config },
        { .command = "restart",  .help = "Restart the device", .func = cmd_restart },
        { .command = "reboot",   .help = "Restart the device (alias)", .func = cmd_restart },
        { .command = "reset",    .help = "Factory reset and restart", .func = cmd_reset },
        { .command = "factory",  .help = "Factory reset and restart (alias)", .func = cmd_reset },
        { .command = "protocol", .help = "Show/set: protocol [auto|mavlink|msp|nmea|none]", .func = cmd_protocol },
        { .command = "heap",     .help = "Show heap memory info", .func = cmd_heap },
        { .command = "log_level",.help = "Set log level: log_level <tag> <LEVEL>", .func = cmd_log_level },
        { .command = "patrol",   .help = "Toggle demo patrol mode: patrol [on|off]", .func = cmd_patrol },
        { .command = "transmit", .help = "Show/set TX modes: transmit <mode> <on|off>", .func = cmd_transmit },
        { .command = "mac",      .help = "Show MAC addresses", .func = cmd_mac },
        { .command = "uptime",   .help = "Show system uptime", .func = cmd_uptime },
    };
    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        esp_console_cmd_register(&cmds[i]);
    }
}

static void cli_task(void *arg)
{
    printf("\n  ESP DRONE REMOTEID CLI\n");
    printf("  Type 'help' for commands, Ctrl+C to exit\n\n");

    while (1) {
        char *line = linenoise("rid> ");
        if (line == NULL) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        linenoiseHistoryAdd(line);
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unknown command: %s  (type 'help')\n", line);
        } else if (err == ESP_ERR_INVALID_ARG) {
            printf("Command error\n");
        }
        linenoiseFree(line);
    }
}

void cli_init(void)
{
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    linenoiseSetMultiLine(1);
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);
    linenoiseHistorySetMaxLen(20);

    register_commands();

    xTaskCreate(cli_task, "cli_task", CLI_STACK_SIZE, NULL, 5, NULL);
    ESP_LOGI(TAG, "CLI initialized");
}
