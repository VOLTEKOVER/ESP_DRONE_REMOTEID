/* -*- tab-width: 2; mode: c; -*-
 *
 * C++ class for OpenDroneID on ESP-IDF
 * This file has the WiFi beacon setup code.
 *
 * Copyright (c) 2023, Steve Jack.
 *
 * MIT licence.
 *
 */

#pragma GCC diagnostic warning "-Wunused-variable"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "id_open.h"

#if ID_OD_WIFI_BEACON && (!USE_BEACON_FUNC)

/*
 * The variables setup by the following function are defined in id_open.h.
 * Some of the tags are copied from beacon frames sent by the Raspberry Pi
 * which is known to work with Android ID apps.
 *
 */

// WiFi beacon frame structure
struct __attribute__((__packed__)) beacon_header {
    uint8_t control[2];          //  0-1:  frame control  
    uint8_t duration[2];         //  2-3:  duration
    uint8_t dest_addr[6];        //  4-9:  destination
    uint8_t src_addr[6];         // 10-15: source  
    uint8_t bssid[6];            // 16-21: base station
    uint8_t seq[2];              // 22-23: sequence
    uint8_t timestamp[8];        // 24-31: 
    uint8_t interval[2];         // 32-33: beacon interval
    uint8_t capability[2];       // 34-35: capability info
};

#endif /* ID_OD_WIFI_BEACON && (!USE_BEACON_FUNC) */
