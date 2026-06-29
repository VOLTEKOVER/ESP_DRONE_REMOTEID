#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_ws2812.h"

#define TAG "WS2812"

#define WS2812_T0H_NS 350
#define WS2812_T0L_NS 800
#define WS2812_T1H_NS 800
#define WS2812_T1L_NS 450
#define WS2812_RESET_US 80

static int g_gpio = -1;
static uint8_t g_brightness = 255;
static bool g_initialized = false;
static rmt_channel_handle_t g_rmt_chan = NULL;
static rmt_encoder_handle_t g_bytes_encoder = NULL;
static rmt_encoder_handle_t g_copy_encoder = NULL;

void led_ws2812_init(int gpio, uint8_t brightness_pct)
{
    if (gpio < 0) return;

    g_gpio = gpio;
    g_brightness = (uint8_t)((uint16_t)brightness_pct * 255 / 100);

    rmt_tx_channel_config_t tx_chan_cfg = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 1,
        .flags.invert_out = false,
        .flags.with_dma = false,
    };
    if (rmt_new_tx_channel(&tx_chan_cfg, &g_rmt_chan) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel");
        return;
    }

    rmt_bytes_encoder_config_t bytes_cfg = {
        .bit0 = {
            .duration0 = WS2812_T0H_NS,
            .level0 = 1,
            .duration1 = WS2812_T0L_NS,
            .level1 = 0,
        },
        .bit1 = {
            .duration0 = WS2812_T1H_NS,
            .level0 = 1,
            .duration1 = WS2812_T1L_NS,
            .level1 = 0,
        },
        .flags.msb_first = 1,
    };
    rmt_new_bytes_encoder(&bytes_cfg, &g_bytes_encoder);

    rmt_copy_encoder_config_t copy_cfg = {};
    rmt_new_copy_encoder(&copy_cfg, &g_copy_encoder);

    rmt_enable(g_rmt_chan);

    g_initialized = true;
    ESP_LOGI(TAG, "WS2812 init on GPIO %d, brightness %d%%", gpio, brightness_pct);
}

void led_ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    if (!g_initialized) return;

    uint8_t scaled_r = (uint16_t)r * g_brightness / 255;
    uint8_t scaled_g = (uint16_t)g * g_brightness / 255;
    uint8_t scaled_b = (uint16_t)b * g_brightness / 255;

    uint8_t grb[3] = { scaled_g, scaled_r, scaled_b };

    rmt_transmit_config_t tx_cfg = {
        .loop_count = 0,
        .flags.eot_level = 0,
    };
    rmt_transmit(g_rmt_chan, g_bytes_encoder, grb, 3, &tx_cfg);
}

void led_ws2812_set_hsv(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t r, g, b;
    uint8_t region = hue / 43;
    uint16_t remainder = (hue - region * 43) * 6;
    uint8_t p = (uint16_t)val * (255 - sat) / 255;
    uint8_t q = (uint16_t)val * (255 - (uint16_t)sat * remainder / 255) / 255;
    uint8_t t = (uint16_t)val * (255 - (uint16_t)sat * (255 - remainder) / 255) / 255;

    switch (region) {
    case 0: r = val; g = t; b = p; break;
    case 1: r = q; g = val; b = p; break;
    case 2: r = p; g = val; b = t; break;
    case 3: r = p; g = q; b = val; break;
    case 4: r = t; g = p; b = val; break;
    default: r = val; g = p; b = q; break;
    }

    led_ws2812_set_rgb(r, g, b);
}

void led_ws2812_set_brightness(uint8_t pct)
{
    g_brightness = (uint8_t)((uint16_t)pct * 255 / 100);
}
