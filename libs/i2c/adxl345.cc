#include "adxl345.hh"

static uint8_t buffer[6];

static const int RATE_DEFAULT[2]     = {100, 0x0A};
static const int RATE_SETABLE[10][2] = {
    {   3, 0x05},
    {   6, 0x06},
    {  12, 0x07},
    {  25, 0x08},
    {  50, 0x09},
    { 100, 0x0A},
    { 200, 0x0B},
    { 400, 0x0C},
    { 800, 0x0D},
    {1600, 0x00}
};


static const int RANGE_DEFAULT[2]    = {2, 0x00};
static const int RANGE_SETABLE[4][2] = {
    { 2, 0x00},
    { 4, 0x01},
    { 8, 0x02},
    {16, 0x03}
};

I2C::ADXL345::ADXL345(char* bus): I2C::Device(bus, I2C::ADXL345::ADDRESS) {
    this->write(0x2D, 0x00);
    this->write(0x2D, 0x08);

    this->range = RANGE_DEFAULT[0];
    this->rate  = RATE_DEFAULT[0];

    this->gain = I2C_ADXL345_MULTIPLICATOR * this->range;
}

I2C::ADXL345::~ADXL345(void) {
    this->write(0x2D, 0x10);
}

void I2C::ADXL345::configure(int rate, int range) {
    size_t i;

    for(i = 0; RATE_SETABLE[i]; ++i) 
        if(rate == RATE_SETABLE[i][0]) {
            this->rate = rate;
            this->write(
                I2C::ADXL345::RATE_ADDRESS,
                RATE_SETABLE[i][1]
            );
        }

    for(i = 0; RANGE_SETABLE[i]; ++i) 
        if(range == RANGE_SETABLE[i][0]) {
            this->range = range;
            this->write(
                I2C::ADXL345::RANGE_ADDRESS,
                RANGE_SETABLE[i][1]
            );
        }

    this->gain = I2C_ADXL345_MULTIPLICATOR * range;
}

float* I2C::ADXL345::read_acceleration(void) {
    float* values = (float*) malloc(3 * sizeof(float));

    this->read(0x32, buffer, 6);

    values[0] = (short) (buffer[1] << 8 | buffer[0])*this->gain;
    values[1] = (short) (buffer[3] << 8 | buffer[2])*this->gain;
    values[2] = (short) (buffer[5] << 8 | buffer[4])*this->gain;

    return values;
}
