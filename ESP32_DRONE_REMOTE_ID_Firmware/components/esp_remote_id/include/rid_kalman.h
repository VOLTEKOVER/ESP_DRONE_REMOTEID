#ifndef RID_KALMAN_H
#define RID_KALMAN_H

#include <stdint.h>
#include <stdbool.h>

#define RID_KALMAN_TIMEOUT_US 3000000

typedef struct {
    float x;
    float v;
    float p00;
    float p01;
    float p11;
    float q_pos;
    float q_vel;
    float r;
    uint64_t t_us;
    bool valid;
} rid_kalman_1d_t;

typedef struct {
    rid_kalman_1d_t lat;
    rid_kalman_1d_t lon;
    rid_kalman_1d_t alt;
} rid_kalman_3d_t;

void rid_kalman_init_1d(rid_kalman_1d_t *k, float q_pos, float q_vel, float r);
void rid_kalman_init(rid_kalman_3d_t *k);
void rid_kalman_update_1d(rid_kalman_1d_t *k, float meas, uint64_t now_us);
void rid_kalman_update(rid_kalman_3d_t *k, double lat, double lon, float alt, uint64_t now_us);
void rid_kalman_predict_1d(rid_kalman_1d_t *k, uint64_t now_us);
void rid_kalman_predict(rid_kalman_3d_t *k, uint64_t now_us);
void rid_kalman_get(const rid_kalman_3d_t *k, double *lat, double *lon, float *alt, float *speed, float *climb, int16_t *heading);
bool rid_kalman_valid(const rid_kalman_3d_t *k);
bool rid_kalman_valid_age(const rid_kalman_3d_t *k, uint64_t now_us);
void rid_kalman_reset(rid_kalman_3d_t *k);

#endif
