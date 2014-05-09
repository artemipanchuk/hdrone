#include "l3g4200d.hh"

uint8_t buffer[6];

static const int RATE_DEFAULT[2]    = {250, 0x00};
static const int RATE_SETABLE[3][2] = {
    { 250, 0x00},
    { 500, 0x10},
    {2000, 0x20}
};

I2C::L3G4200D::L3G4200D(char* bus): I2C::Device(bus, I2C::L3G4200D::ADDRESS) {
    this->write(0x20, 0x1F);
    this->write(0x21, 0x00);
    this->write(0x22, 0x08);
    this->write(0x24, 0x00);
    
    this->rate = RATE_DEFAULT[0];

    this->gain = this->rate * I2C_L3G4200D_MULTIPLICATOR;
}

I2C::L3G4200D::~L3G4200D(void) {
    this->write(0x20, 0x00);
}

void I2C::L3G4200D::configure(int rate, int range) {
    size_t i;

    for(i = 0; RATE_SETABLE[i]; ++i) 
        if(rate == RATE_SETABLE[i][0]) {
            this->rate = rate;
            this->write(
                I2C::L3G4200D::RATE_ADDRESS,
                RATE_SETABLE[i][1]
            );
        }

    this->gain = I2C_L3G4200D_MULTIPLICATOR * rate;
}

float* I2C::L3G4200D::read_velocity(void) {
    float* values = (float*) malloc(3 * sizeof(float));

    this->read(0x80|0x28, buffer, 6);

    values[0] = (short) (buffer[1] << 8 | buffer[0])*this->gain;
    values[1] = (short) (buffer[3] << 8 | buffer[2])*this->gain;
    values[2] = (short) (buffer[5] << 8 | buffer[4])*this->gain;

    return values;
}
