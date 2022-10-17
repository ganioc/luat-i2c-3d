#ifndef KALMAN_H
#define KALMN_H

#include "core_api.h"

/**
 * @brief Kalman Filter
 * Default parameters: 0.1, 20, 1.0, 0, 1.0
 */

#define KALMAN_R 0.05
#define KALMAN_Q 20
#define KALMAN_A 1.0
#define KALMAN_B 0
#define KALMAN_C 1.0

typedef struct {
    double R; // Process noise, 1
    double Q; // Measurement noise, 1
    double A; // State vector, 1
    double B; // Control vector, 0
    double C; // Measurement vector , 1
    double cov;
    double x;
    uint8  first_time; // 0
} Kalman_t;

void Kalman_init(Kalman_t * filter, double R, double Q, double A, double B, double C);

/**
 * @brief 
 * Filter a new value,
 * @param z measurement
 * @param u Control
 * @return double 
 */
double Kalman_filter(Kalman_t *filter, double z, double u);

// double Kalman_predict(Kalman_t *filter, double u);
// double Kalman_uncertainty(Kalman_t *filter);
// double Kalman_lastMeasurement(Kalman_t *filter);
// double Kalman_setMeasurementNoise(Kalman_t *filter, double noise);
// double Kalman_setProcessNoise(Kalman_t *filter, double noise);

#endif