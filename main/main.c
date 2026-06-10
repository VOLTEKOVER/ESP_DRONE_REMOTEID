#include <stdio.h>
#include "esp_log.h"
#include "esp_remote_id.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP Remote ID v%s starting", ESP_RID_VERSION);

    esp_rid_init();
    esp_rid_start();

    ESP_LOGI(TAG, "ESP Remote ID running");
    ESP_LOGI(TAG, "Connect to WiFi AP 'ESP-RID' and open http://192.168.4.1");
}
