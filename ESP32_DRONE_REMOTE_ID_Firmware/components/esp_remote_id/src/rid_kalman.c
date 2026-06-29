#include <math.h>
#include "rid_kalman.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define DEG2M_LAT 111320.0f

void rid_kalman_init_1d(rid_kalman_1d_t *k, float q_pos, float q_vel, float r)
{
    k->x = 0.0f;
    k->v = 0.0f;
    k->p00 = 0.0f;
    k->p01 = 0.0f;
    k->p11 = 0.0f;
    k->q_pos = q_pos;
    k->q_vel = q_vel;
    k->r = r;
    k->t_us = 0;
    k->valid = false;
}

void rid_kalman_init(rid_kalman_3d_t *k)
{
    rid_kalman_init_1d(&k->lat, 1e-9f, 1e-8f, 1e-9f);
    rid_kalman_init_1d(&k->lon, 1.5e-9f, 1.5e-8f, 1.5e-9f);
    rid_kalman_init_1d(&k->alt, 1.0f, 10.0f, 25.0f);
}

void rid_kalman_predict_1d(rid_kalman_1d_t *k, uint64_t now_us)
{
    if (!k->valid) return;

    int64_t dt_us = (int64_t)(now_us - k->t_us);
    if (dt_us <= 0) return;

    float dt = (float)dt_us * 1e-6f;

    k->x += k->v * dt;
    float p00 = k->p00 + 2.0f * k->p01 * dt + k->p11 * dt * dt + k->q_pos * dt;
    float p01 = k->p01 + k->p11 * dt;
    float p11 = k->p11 + k->q_vel * dt;
    k->p00 = p00;
    k->p01 = p01;
    k->p11 = p11;

    k->t_us = now_us;
}

void rid_kalman_predict(rid_kalman_3d_t *k, uint64_t now_us)
{
    rid_kalman_predict_1d(&k->lat, now_us);
    rid_kalman_predict_1d(&k->lon, now_us);
    rid_kalman_predict_1d(&k->alt, now_us);
}

void rid_kalman_update_1d(rid_kalman_1d_t *k, float meas, uint64_t now_us)
{
    if (!k->valid) {
        k->x = meas;
        k->v = 0.0f;
        k->p00 = k->r;
        k->p01 = 0.0f;
        k->p11 = 100.0f * k->r;
        k->t_us = now_us;
        k->valid = true;
        return;
    }

    rid_kalman_predict_1d(k, now_us);

    float y = meas - k->x;
    float s = k->p00 + k->r;
    float k0 = k->p00 / s;
    float k1 = k->p01 / s;

    k->x += k0 * y;
    k->v += k1 * y;

    float p00_new = (1.0f - k0) * k->p00;
    float p01_new = (1.0f - k0) * k->p01;
    float p11_new = k->p11 - k1 * k->p01;
    k->p00 = p00_new;
    k->p01 = p01_new;
    k->p11 = p11_new;

    k->t_us = now_us;
}

void rid_kalman_update(rid_kalman_3d_t *k, double lat, double lon, float alt, uint64_t now_us)
{
    rid_kalman_update_1d(&k->lat, (float)lat, now_us);
    rid_kalman_update_1d(&k->lon, (float)lon, now_us);
    rid_kalman_update_1d(&k->alt, alt, now_us);
}

void rid_kalman_get(const rid_kalman_3d_t *k, double *lat, double *lon, float *alt,
                    float *speed, float *climb, int16_t *heading)
{
    if (lat) *lat = (double)k->lat.x;
    if (lon) *lon = (double)k->lon.x;
    if (alt) *alt = k->alt.x;

    if (speed || climb || heading) {
        float lat_rad = k->lat.x * (float)(M_PI / 180.0);
        float cos_lat = cosf(lat_rad);
        if (cos_lat < 0.01f) cos_lat = 0.01f;

        float vn = k->lat.v * DEG2M_LAT;
        float ve = k->lon.v * DEG2M_LAT * cos_lat;

        if (speed) *speed = sqrtf(vn * vn + ve * ve);
        if (climb) *climb = k->alt.v;

        if (heading) {
            float h = atan2f(ve, vn) * (180.0f / (float)M_PI);
            if (h < 0.0f) h += 360.0f;
            if (h >= 360.0f) h -= 360.0f;
            *heading = (int16_t)(h + 0.5f);
        }
    }
}

bool rid_kalman_valid(const rid_kalman_3d_t *k)
{
    return k->lat.valid || k->lon.valid || k->alt.valid;
}

bool rid_kalman_valid_age(const rid_kalman_3d_t *k, uint64_t now_us)
{
    if (!k->lat.valid && !k->lon.valid && !k->alt.valid)
        return false;
    if (now_us - k->lat.t_us > RID_KALMAN_TIMEOUT_US ||
        now_us - k->lon.t_us > RID_KALMAN_TIMEOUT_US ||
        now_us - k->alt.t_us > RID_KALMAN_TIMEOUT_US)
        return false;
    return true;
}

void rid_kalman_reset(rid_kalman_3d_t *k)
{
    k->lat.valid = false;
    k->lon.valid = false;
    k->alt.valid = false;
}
