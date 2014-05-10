#include "bmp085.hh"

I2C::BMP085::BMP085(char* bus): I2C::Device(bus, I2C::BMP085::ADDRESS) {
    uint8_t buffer[2];

    this->read(0xAA, buffer, 2);
    this->ac1 = buffer[0] << 8 | buffer[1]; 
    
    this->read(0xAC, buffer, 2);
    this->ac2 = buffer[0] << 8 | buffer[1];
    
    this->read(0xAE, buffer, 2);
    this->ac3 = buffer[0] << 8 | buffer[1];
    
    this->read(0xB6, buffer, 2);
    this->b1 = buffer[0] << 8 | buffer[1];
    
    this->read(0xB8, buffer, 2);
    this->b2 = buffer[0] << 8 | buffer[1];
    
    this->read(0xBA, buffer, 2);
    this->mb = buffer[0] << 8 | buffer[1];
    
    this->read(0xBC, buffer, 2);
    this->mc = buffer[0] << 8 | buffer[1];
    
    this->read(0xBE, buffer, 2);
    this->md = buffer[0] << 8 | buffer[1];

    this->read(0xB0, buffer, 2);
    this->ac4 = buffer[0] << 8 | buffer[1];

    this->read(0xB2, buffer, 2);
    this->ac5 = buffer[0] << 8 | buffer[1];

    this->read(0xB4, buffer, 2);
    this->ac6 = buffer[0] << 8 | buffer[1];

    this->read_temperature();
}

double I2C::BMP085::read_temperature(void) {
    uint32_t ut          = this->read_ut();
    double   temperature = this->calc_temperature(ut);

    return temperature;
}

double I2C::BMP085::read_pressure(void) {
    uint32_t up       = this->read_up();
    double   pressure = this->calc_pressure(up);

    return pressure;
}

double I2C::BMP085::read_altitude(void) {
    uint32_t up       = this->read_up();
    double   pressure = this->calc_pressure(up);
    double   altitude = this->calc_altitude(pressure);

    return altitude;
}

int32_t I2C::BMP085::read_ut(void) {
    uint8_t buffer[2];
    int32_t ut = 0;

    this->write(0xF4, 0x2E);
       
    usleep(5e3);
    
    this->read(0xF6, buffer, 2);
    ut = buffer[0] << 8 | buffer[1];
       
    return ut;
}

int32_t I2C::BMP085::read_up(void) {
    uint8_t buffer[3];
    int32_t up = 0;

    this->write(0xF4, 0x34 + (I2C_BMP085_OVERSAMPLING << 6));

    usleep((2 + (3 << I2C_BMP085_OVERSAMPLING))*1e3);

    this->read(0xF6, buffer, 3);

    up = (((uint16_t) buffer[0] << 16) | ((uint16_t) buffer[1] <<  8) | (uint16_t) buffer[2])
         >> (8 - I2C_BMP085_OVERSAMPLING);

    return up;
}

double I2C::BMP085::calc_temperature(uint32_t ut) {
   int32_t x1, x2;

   x1 = ((ut - this->ac6) * this->ac5) >> 15;
   x2 = (this->mc << 11)/(x1 + this->md);
   this->b5 = x1 + x2;

   return (double) ((this->b5 + 8) >> 4)/10.0;
}

double I2C::BMP085::calc_pressure(uint32_t up) {
    int32_t  x1, x2, x3, b3, b6, p;
    uint32_t b4, b7;

    b6 = this->b5 - 4000;

    x1 = (this->b2  * ((b6*b6) >> 12)) >> 11;
    x2 = (this->ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = (((this->ac1*4 + x3) << I2C_BMP085_OVERSAMPLING) + 2) >> 2;

    x1 = (this->ac3 * b6) >> 13;
    x2 = (this->b1  * ((b6*b6) >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;
    b4 = (this->ac4 * (uint32_t) (x3 + 32768)) >> 15;

    b7 = ((uint32_t) up - b3) * (50000 >> I2C_BMP085_OVERSAMPLING);
    if(b7 < 0x80000000)
        p = (b7 << 1)/b4;
    else
        p = (b7/b4) << 1;

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p += (x1 + x2 + 3791) >> 4;

    return (double) p;
}
