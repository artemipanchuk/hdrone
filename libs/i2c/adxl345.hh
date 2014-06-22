#include "device.hh"

#ifndef I2C_ADXL345_H
#define I2C_ADXL345_H

#define I2C_ADXL345_MULTIPLICATOR (2.0/1024.0)

namespace I2C {
    class ADXL345: public Device {
    public:
         ADXL345(const char* bus);
        ~ADXL345(void);
        
        void   configure(int rate, int range);
        float* read_acceleration(void);

    private:
        static const uint8_t ADDRESS = 0x53;
        static const uint8_t RATE_ADDRESS  = 0x2C;
        static const uint8_t RANGE_ADDRESS = 0x31;

        float gain;

        int rate;
        int range;
    };
}

#endif
