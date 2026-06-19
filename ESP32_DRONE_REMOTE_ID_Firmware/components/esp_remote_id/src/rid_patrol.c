#include <math.h>
#include "rid_patrol.h"

#define PATROL_HOME_LAT 41.9028
#define PATROL_HOME_LON 12.4964
#define PATROL_RADIUS   0.003
#define PATROL_SPEED    6.0

void rid_patrol_tick(rid_gps_data_t *gps)
{
    static float angle = 0.0f;
    angle += 0.018f;
    if (angle > 2.0f * M_PI) angle -= 2.0f * M_PI;

    gps->latitude  = PATROL_HOME_LAT + cosf(angle) * PATROL_RADIUS;
    gps->longitude = PATROL_HOME_LON + sinf(angle) * PATROL_RADIUS;
    gps->altitude_msl     = 50.0f + sinf(angle * 2.0f) * 20.0f;
    gps->altitude_baro    = gps->altitude_msl;
    gps->altitude_relative = gps->altitude_msl;
    gps->speed       = PATROL_SPEED + sinf(angle) * 2.0f;
    gps->speed_vertical = sinf(angle * 2.0f) * 0.5f;
    gps->heading     = (int16_t)(atan2f(cosf(angle), -sinf(angle)) * 180.0f / M_PI);
    if (gps->heading < 0) gps->heading += 360;

    float var = sinf(angle * 3.0f) * 0.5f + 0.5f;
    gps->fix_type   = (uint8_t)(2 + (uint8_t)(var * 2.0f));
    if (gps->fix_type > 4) gps->fix_type = 4;
    if (gps->fix_type < 2) gps->fix_type = 2;
    gps->satellites = (uint8_t)(6 + (uint8_t)(var * 10.0f));
    gps->armed      = true;
}
