#ifndef RID_LIGHTING_H
#define RID_LIGHTING_H

#include <stdint.h>

#define RID_LIGHTING_MAX_OUTPUTS 5

void rid_lighting_init(const int8_t pins[RID_LIGHTING_MAX_OUTPUTS],
                       const uint8_t patterns[RID_LIGHTING_MAX_OUTPUTS],
                       const int16_t phase_offsets[RID_LIGHTING_MAX_OUTPUTS]);
void rid_lighting_tick(void);
void rid_lighting_set_state(bool armed, bool gps_valid);

#endif
