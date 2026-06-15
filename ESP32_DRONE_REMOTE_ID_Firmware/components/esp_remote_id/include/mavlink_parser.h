#ifndef MAVLINK_PARSER_H
#define MAVLINK_PARSER_H

#include "esp_remote_id.h"

void mavlink_parser_init(void);
bool mavlink_parser_get(rid_gps_data_t *gps);
bool mavlink_parser_get_identity(rid_identity_t *identity);
void mavlink_parser_set_sysid_filter(uint8_t sysid);

#endif
