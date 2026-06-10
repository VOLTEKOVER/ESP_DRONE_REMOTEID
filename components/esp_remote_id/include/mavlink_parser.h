#ifndef MAVLINK_PARSER_H
#define MAVLINK_PARSER_H

#include "esp_remote_id.h"

void mavlink_parser_init(void);
bool mavlink_parser_get(rid_gps_data_t *gps);

#endif
