#ifndef PROTOCOL_DETECT_H
#define PROTOCOL_DETECT_H

#include "esp_remote_id.h"

void protocol_detect_init(void);
rid_protocol_t protocol_detect_auto(void);
void protocol_detect_set_fixed(rid_protocol_t proto);

#endif
