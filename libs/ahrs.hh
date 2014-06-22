#include <stdint.h>
#include <cstdio>
#include <cmath>

#include "i2c/adxl345.hh"
#include "i2c/bmp085.hh"
#include "i2c/hmc5883l.hh"
#include "i2c/l3g4200d.hh"
#include "quaternion.hh"

#ifndef AHRS_H
#define AHRS_H

class AHRS {
public:
    Quaternion* attitude;
    
    AHRS(const char*, float, uint16_t);

    float gx, gy, gz;
    float ax, ay, az;
    float mx, my, mz;
    float gravity;

    void update(void);

private:
    I2C::ADXL345  *a;
    I2C::BMP085   *b;
    I2C::L3G4200D *g;
    I2C::HMC5883L *m;
    
    float    gain;
    uint16_t delay;
};


#endif
