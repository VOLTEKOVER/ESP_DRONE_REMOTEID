/*
 * OpenDroneID wrapper header for ESP-IDF project
 * 
 * This header provides access to the OpenDroneID libraries
 * from the id_open component.
 * 
 * Converted to pure ESP-IDF (no Arduino dependencies)
 */

#ifndef OPENDRONEID_H
#define OPENDRONEID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Include the actual OpenDroneID definitions
#include "id_open.h"

#ifdef __cplusplus
}
#endif

#endif // OPENDRONEID_H

