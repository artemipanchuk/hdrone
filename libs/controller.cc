#include "controller.hh"

static AHRS* ahrs;

static PWM::Afro_ESC* motor_x;
static PWM::Afro_ESC* motor_nx;
static PWM::Afro_ESC* motor_y;
static PWM::Afro_ESC* motor_ny;

static float  x_corrections[4] = {-1, 0,  0.5, 1};
static float nx_corrections[4] = { 1, 0, -0.5, 1};
static float  y_corrections[4] = { 0,-1,  0.5, 1};
static float ny_corrections[4] = { 0, 1, -0.5, 1};

static Quaternion* calculate_error(Quaternion* attitude, Quaternion* target) {
    Quaternion* attitude_x = new Quaternion(attitude);

    attitude_x->invert();
    attitude_x->multiply(target);

    return attitude_x;
}

static float calculate_amount(float error, float rate, float kp, float kd) {
  return (kp * error) + (kd * -rate);
}

v8::Handle<v8::Value> update(const v8::Arguments& args) {
    v8::HandleScope scope;

    if(args.Length() < 5) {
        v8::ThrowException(v8::Exception::TypeError(v8::String::New("Wrong number of arguments")));
        return scope.Close(v8::Undefined());
    }

    if (!args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber()) {
        v8::ThrowException(v8::Exception::TypeError(v8::String::New("Wrong type of arguments")));
        return scope.Close(v8::Undefined());
    }

    ahrs->update();

    Quaternion* target = new Quaternion();
    target->w = (float) args[0]->NumberValue();
    target->i = (float) args[1]->NumberValue();
    target->j = (float) args[2]->NumberValue();
    target->k = (float) args[3]->NumberValue();

    float altitude_trigger = (float) args[4]->NumberValue();

    Quaternion* error = calculate_error(ahrs->attitude, target);

    float    pitch_amount = calculate_amount(error->rot_y(), ahrs->gy, PITCH_KP, PITCH_KD);
    float     roll_amount = calculate_amount(error->rot_x(), ahrs->gx,  ROLL_KP,  ROLL_KD);
    float      yaw_amount = calculate_amount(error->rot_z(), ahrs->gz,   YAW_KP,   YAW_KD);
    float altitude_amount = ahrs->gravity + 256 + altitude_trigger;

    motor_x->add_pitch   (   pitch_amount);
    motor_x->add_roll    (    roll_amount);
    motor_x->add_yaw     (     yaw_amount);
    motor_x->add_altitude(altitude_amount);

    motor_nx->add_pitch   (   pitch_amount);
    motor_nx->add_roll    (    roll_amount);
    motor_nx->add_yaw     (     yaw_amount);
    motor_nx->add_altitude(altitude_amount);

    motor_y->add_pitch   (   pitch_amount);
    motor_y->add_roll    (    roll_amount);
    motor_y->add_yaw     (     yaw_amount);
    motor_y->add_altitude(altitude_amount);

    motor_ny->add_pitch   (   pitch_amount);
    motor_ny->add_roll    (    roll_amount);
    motor_ny->add_yaw     (     yaw_amount);
    motor_ny->add_altitude(altitude_amount);

    return scope.Close(v8::Undefined());
}

void initiation(v8::Handle<v8::Object> exports) {
    ahrs = new AHRS("/dev/i2c-0", 0.5, 500);

    motor_x  = new PWM::Afro_ESC(0, 18, x_corrections );
    motor_nx = new PWM::Afro_ESC(1, 23, nx_corrections);
    motor_y  = new PWM::Afro_ESC(2, 24, y_corrections );
    motor_ny = new PWM::Afro_ESC(3, 25, ny_corrections);

    exports->Set(
        v8::String::NewSymbol("update"),
        v8::FunctionTemplate::New(update)->GetFunction()
    );
}

NODE_MODULE(controller, initiation)
