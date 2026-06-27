#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <stdint.h>
#include <stdbool.h>

#define RID_LED_PIN_NC  -1

#ifndef CONFIG_RID_LED_R_GPIO
#define CONFIG_RID_LED_R_GPIO RID_LED_PIN_NC
#endif
#ifndef CONFIG_RID_LED_G_GPIO
#define CONFIG_RID_LED_G_GPIO RID_LED_PIN_NC
#endif
#ifndef CONFIG_RID_LED_B_GPIO
#define CONFIG_RID_LED_B_GPIO RID_LED_PIN_NC
#endif

typedef enum {
    RID_LED_BOOT,
    RID_LED_NO_GPS,
    RID_LED_GPS_OK,
    RID_LED_DEMO,
    RID_LED_LOCKED,
    RID_LED_OTA,
    RID_LED_ERROR,
} rid_led_state_t;

void led_status_init(void);
void led_status_reconfigure(int r_pin, int g_pin, int b_pin);
void led_status_set_state(rid_led_state_t state);
void led_status_tx_flash(void);
void led_status_tick(void);

#endif
