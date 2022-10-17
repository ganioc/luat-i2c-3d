#include "../include/kalman.h"

/**
 * @brief 
 * 
 * @param filter 
 * @param u Control, default is 0
 * @return double 
 */
double Kalman_predict(Kalman_t *filter, double u){
    double u1 = u;
    return (filter->A*filter->x + filter->B*u1);
}
double Kalman_uncertainty(Kalman_t *filter){
    return (filter->A*filter->cov *filter->A + filter->R);
}
double Kalman_lastMeasurement(Kalman_t *filter){
    return filter->x;
}
double Kalman_setMeasurementNoise(Kalman_t *filter, double noise){
    filter->Q = noise;
}
double Kalman_setProcessNoise(Kalman_t *filter, double noise){
    filter->R = noise;
}

void Kalman_init(Kalman_t * filter, double R, double Q, double A, double B, double C){
    filter->R = R;
    filter->Q = Q;
    filter->A = A;
    filter->B = B;
    filter->C = C;

    filter->first_time = 0;
}

double Kalman_filter(Kalman_t *filter, double z, double u){
    double predX, predCov, K;

    if(filter->first_time == 0){
        filter->first_time = 1;
        filter->x = (1/filter->C)*z;
        filter->cov = (1/filter->C)*filter->Q*(1/filter->C);

    }else{
        // Compute prediction
        predX = Kalman_predict(filter, u);
        predCov = Kalman_uncertainty(filter);

        // Kalman gain
        K = predCov*filter->C*(1/(filter->C*predCov*filter->C + filter->Q));

        // Correction,
        filter->x = predX + K * (z - filter->C*predX);
        filter->cov = predCov - (K*filter->C*predCov);

    }
    return filter->x;
}

