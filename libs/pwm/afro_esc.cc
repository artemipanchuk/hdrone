#include "afro_esc.hh"

PWM::Afro_ESC::Afro_ESC(int motor_id, int pin_id): PWM::Device(motor_id, pin_id)  {
    this->set(0);
}

void PWM::Afro_ESC::set(int value) {
    if((value > 100) || (value < 0)) {
        printf("value should be between 0 and 100 (PWM::Afro_ESC::set). Ignoring.\n");
        return;
    }

    if(value == 0)
        PWM::Device::set(PWM::Afro_ESC::STANDBY);
    else
        PWM::Device::set(PWM::Afro_ESC::MIN + value*PWM::Afro_ESC::RANGE/100);
}
