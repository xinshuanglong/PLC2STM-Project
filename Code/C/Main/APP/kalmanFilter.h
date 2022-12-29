#ifndef _KF_H
#define _KF_H

typedef struct
{
    float LastP;
    float Now_P;
    float out;
    float Kg;
    float Q;
    float R;
} KFP; //Kalman Filter parameter

void kalmanParamInit(KFP *kfp, float P, float Q, float R);
float kalmanFilter(KFP *kfp, float input);

#endif
