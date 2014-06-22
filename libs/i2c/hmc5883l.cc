#include "hmc5883l.hh"

static uint8_t buffer[6];

static const float RATE_DEFAULT[2]    = {15, 0x10};
static const float RATE_SETABLE[7][2] = {
    { 0.75, 0x00},
    { 1.5,  0x04},
    { 3,    0x08},
    { 7.5,  0x0C},
    {15,    0x10},
    {30,    0x14},
    {75,    0x18}
};


static const float RANGE_DEFAULT[2]    = {1.3, 0x20};
static const float RANGE_SETABLE[8][2] = {
    {0.88, 0x00},
    {1.3,  0x20},
    {1.9,  0x40},
    {2.5,  0x60},
    {4,    0x80},
    {4.7,  0xA0},
    {5.6,  0xC0},
    {8.1,  0xE0}
};

I2C::HMC5883L::HMC5883L(const char* bus): I2C::Device(bus, I2C::HMC5883L::ADDRESS) {
    this->write(0x02, 0x00);

    this->rate  = RATE_DEFAULT[0];
    this->range = RANGE_DEFAULT[0];

    this->gain = I2C_HMC5883L_MULTIPLICATOR * this->range + I2C_HMC5883L_SUMMAND;
}

I2C::HMC5883L::~HMC5883L(void) {
    this->write(0x02, 0x02);
}

void I2C::HMC5883L::configure(float rate, float range) {
    size_t i;

    for(i = 0; RATE_SETABLE[i]; ++i) 
        if(rate == RATE_SETABLE[i][0]) {
            this->rate = rate;
            this->write(
                I2C::HMC5883L::RATE_ADDRESS,
                (int) RATE_SETABLE[i][1]
            );
        }

    for(i = 0; RANGE_SETABLE[i]; ++i) 
        if(range == RANGE_SETABLE[i][0]) {
            this->range = range;
            this->write(
                I2C::HMC5883L::RANGE_ADDRESS,
                (int) RANGE_SETABLE[i][1]
            );
        }

    this->gain = I2C_HMC5883L_MULTIPLICATOR * range + I2C_HMC5883L_SUMMAND;
}

float* I2C::HMC5883L::read_induction(void) {
    float* values = (float*) malloc(3 * sizeof(float));

    this->read(0x03, buffer, 6);

    values[0] = (short) (buffer[1] << 8 | buffer[0])*this->gain;
    values[1] = (short) (buffer[3] << 8 | buffer[2])*this->gain;
    values[2] = (short) (buffer[5] << 8 | buffer[4])*this->gain;

    return values;
}
