#include "device.hh"

#ifndef I2C_L3G4200D_H
#define I2C_L3G4200D_H

#define I2C_L3G4200D_MULTIPLICATOR (2.0/65536.0 * 3.1415926/180.0)

namespace I2C {
    class L3G4200D: public Device {
    public:
         L3G4200D(const char* bus);
        ~L3G4200D(void);
        
        void   configure(int rate);
        float* read_velocity(void);

    private:
        static const uint8_t ADDRESS = 0x69;
        static const uint8_t RATE_ADDRESS = 0x23;

        float gain;

        int rate;
    };
}

#endif
