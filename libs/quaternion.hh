#include <stdint.h>
#include <math.h>

#ifndef QUATERNION_H
#define QUATERNION_H

float invsqrt(float);

class Quaternion {
public:    
    float w, i, j, k;

    Quaternion(void);
    Quaternion(Quaternion*);
    
    float pitch(void);
    float roll(void);
    float yaw(void);

    float rot_x(void);
    float rot_y(void);
    float rot_z(void);

    void normalize(void);
    void invert(void);
    void multiply(Quaternion*);
};


#endif
