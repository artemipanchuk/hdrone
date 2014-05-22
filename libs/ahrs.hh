#include <stdint.h>
#include <cstdio>
#include <cmath>

#ifndef AHRS_H
#define AHRS_H

#define AHRS_INVSQRT_INSTABILITY_FIX 2

class AHRS {
public:
    float quaternion[4];

    AHRS(float gain, uint16_t delay);

    void update(float gx, float gy, float gz,
                float ax, float ay, float az,
                float mx, float my, float mz);
private:
    float    gain;
    uint16_t delay;
};


#endif
