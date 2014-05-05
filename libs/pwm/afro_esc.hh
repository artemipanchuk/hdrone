#include "device.hh"

namespace PWM {
    class Afro_ESC: public Device {
    public:
        Afro_ESC(int motor_id, int pin_id);
        void set(int value);
    private:
        const static int STANDBY = 105;
        const static int MIN     = 106;
        const static int MAX     = 186;

        const static int RANGE = 186-106;
    };
}
