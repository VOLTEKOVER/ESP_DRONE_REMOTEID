#ifndef RID_DRONECAN_H
#define RID_DRONECAN_H

#include <stdbool.h>
#include "esp_remote_id.h"

bool rid_dronecan_init(int rx_gpio, int tx_gpio, uint32_t bitrate);
bool rid_dronecan_get(rid_gps_data_t *gps);
bool rid_dronecan_is_active(void);

#endif
