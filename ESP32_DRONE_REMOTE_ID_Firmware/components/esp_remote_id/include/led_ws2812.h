#ifndef LED_WS2812_H
#define LED_WS2812_H

#include <stdint.h>
#include <stdbool.h>

void led_ws2812_init(int gpio, uint8_t brightness_pct);
void led_ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);
void led_ws2812_set_hsv(uint16_t hue, uint8_t sat, uint8_t val);
void led_ws2812_set_brightness(uint8_t pct);

#endif
