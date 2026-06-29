#ifndef RID_OTA_H
#define RID_OTA_H

#include <stdbool.h>
#include "esp_remote_id.h"

bool rid_ota_check_and_run(rid_config_t *cfg);

#endif
