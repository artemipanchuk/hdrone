#include "device.hh"

#include <cmath>

#ifndef I2C_BMP085_H
#define I2C_BMP085_H

#define I2C_BMP085_OVERSAMPLING 3

namespace I2C {
    class BMP085: public Device {
    public:
        BMP085(char* bus);
        
        double read_temperature(void);
        double read_pressure(void);

    private:
        static const uint8_t ADDRESS = 0x77;

        int16_t  ac1, ac2, ac3, b1, b2, mb, mc, md;
        uint16_t ac4, ac5, ac6;

        int32_t b5;

        int32_t read_ut(void);
        int32_t read_up(void);

        double calc_temperature(uint32_t ut);
        double calc_pressure(uint32_t up);
    };
}

#endif
