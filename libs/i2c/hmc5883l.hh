#include "device.hh"

#ifndef I2C_HMC5883L_H
#define I2C_HMC5883L_H

#define I2C_HMC5883L_MULTIPLICATOR (2.0/4096.0)
#define I2C_HMC5883L_SUMMAND       (0.0003)

namespace I2C {
    class HMC5883L: public Device {
    public:
         HMC5883L(const char* bus);
        ~HMC5883L(void);
        
        void   configure(float rate, float range);
        float* read_induction(void);

    private:
        static const uint8_t ADDRESS = 0x1E;
        static const uint8_t RATE_ADDRESS  = 0x00;
        static const uint8_t RANGE_ADDRESS = 0x01;

        float gain;

        int rate;
        int range;
    };
}

#endif
