#include "afro_esc.hh"

PWM::Afro_ESC::Afro_ESC(int motor_id, int pin_id, float* corrections): PWM::Device(motor_id, pin_id)  {
    this->pitch    = corrections[0];
    this->roll     = corrections[1];
    this->yaw      = corrections[2];
    this->altitude = corrections[3];

    PWM::Device::set(STANDBY);

    this->value = 0;
}

void PWM::Afro_ESC::update(void) {
    if((value > 100) || (value < 0)) {
        printf("[WARNING] [PWM::Afro_ESC::set] value should be between 0 and 100. Ignoring.\n");
        return;
    }

    if(value == 0)
        return;
    else
        PWM::Device::set(PWM::Afro_ESC::MIN + value*PWM::Afro_ESC::RANGE/100);
}

void PWM::Afro_ESC::add_pitch(int amount) {
  amount *= pitch;

  if (amount > 0)
    amount = 0;

  value += amount*PITCH_SCALE;
}

void PWM::Afro_ESC::add_roll(int amount) {
  amount *= roll;

  if (amount > 0)
    amount = 0;

  value += amount*ROLL_SCALE;
}

void PWM::Afro_ESC::add_yaw(int amount) {
  value += amount*yaw*YAW_SCALE;
}

void PWM::Afro_ESC::add_altitude(int amount) {
  value += amount*altitude*ALTITUDE_SCALE;
}
