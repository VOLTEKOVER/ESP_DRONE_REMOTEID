#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_remote_id.h"
#include "esp_mac.h"
#include "esp_wifi.h"

#define TAG "MAIN"

static void print_splash(void)
{
    uint8_t mac[6] = {0};
    esp_wifi_get_mac(WIFI_IF_AP, mac);

    printf("\n");
    printf("  ╔══════════════════════════════════════════════╗\n");
    printf("  ║                                              ║\n");
    printf("  ║       ___ ____  ____                         ║\n");
    printf("  ║      / __/ __ \\/ __ \\    Remote ID           ║\n");
    printf("  ║     / /_/ /_/ / /_/ /    Transmitter         ║\n");
    printf("  ║     \\__/ .___/ .___/     ————————           ║\n");
    printf("  ║       /_/   /_/                              ║\n");
    printf("  ║                                              ║\n");
    printf("  ║  v%s                               ║\n", ESP_RID_VERSION);
    printf("  ║  ASTM F3411-22a                              ║\n");
    printf("  ║                                              ║\n");
    printf("  ╠══════════════════════════════════════════════╣\n");
    printf("  ║                                              ║\n");
    printf("  ║  WiFi  AP   │  ESP-RID                       ║\n");
    printf("  ║  Config URL │  http://192.168.4.1            ║\n");
    printf("  ║  MAC  AP    │  %02x:%02x:%02x:%02x:%02x:%02x              ║\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("  ║                                              ║\n");
    printf("  ╚══════════════════════════════════════════════╝\n");
    printf("\n");
}

void app_main(void)
{
    esp_rid_init();
    esp_rid_start();

    print_splash();

    ESP_LOGI(TAG, "Connect to WiFi AP 'ESP-RID' and open http://192.168.4.1");
}
