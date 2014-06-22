#include "device.hh"

#ifndef PWM_AFRO_ESC_H
#define PWM_AFRO_ESC_H

#define PITCH_SCALE    10
#define ROLL_SCALE     10
#define YAW_SCALE       7
#define ALTITUDE_SCALE  0.01

namespace PWM {
    class Afro_ESC: public Device {
    public:
        Afro_ESC(int, int, float*);

        float value;

        float pitch;
        float roll;
        float yaw;
        float altitude;

        void update(void);
        void add_pitch(int);
        void add_roll(int);
        void add_yaw(int);
        void add_altitude(int);

    private:
        const static int STANDBY = 105;
        const static int MIN     = 106;
        const static int MAX     = 186;

        const static int RANGE = 186-106;
    };
}

#endif
