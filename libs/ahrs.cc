#include "ahrs.hh"

AHRS::AHRS(const char* i2c_bus, float gain, uint16_t delay) {
    if(gain < 0 || 1 < gain) {
        gain = 0.1;
        printf("[WARNING] [AHRS::AHRS] Invalid gain. Ignoring. Set to 0.1.\n");
    }

    this->gain = gain;

    if(1000 < delay) {
        delay = 100;
        printf("[WARNING] [AHRS::AHRS] Invalid frequency. Ignoring. Set to 100 ms.\n");
    }

    this->delay = delay;

    attitude = new Quaternion();
    a = new I2C::ADXL345 (i2c_bus);
    b = new I2C::BMP085  (i2c_bus);
    m = new I2C::HMC5883L(i2c_bus);
    g = new I2C::L3G4200D(i2c_bus);
}

void AHRS::update(void) {
    float *g_data = g->read_velocity();
    float *a_data = a->read_acceleration();    
    float *m_data = m->read_induction();

    gx = g_data[0]; gy = g_data[1]; gz = g_data[2];
    ax = a_data[0]; ay = a_data[1]; az = a_data[2];
    mx = m_data[0]; my = m_data[1]; mz = m_data[2];

    free(g_data);
    free(a_data);
    free(m_data);

    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot1, q_dot2, q_dot3, q_dot4;
    float hx, hy;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx,
          _2bx, _2bz, _4bx, _4bz,
          _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3,
          q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    float q0 = attitude->w,
          q1 = attitude->i,
          q2 = attitude->j,
          q3 = attitude->k;

    q_dot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    q_dot2 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    q_dot3 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    q_dot4 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        recip_norm = invsqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        recip_norm = invsqrt(mx * mx + my * my + mz * mz);
        mx *= recip_norm;
        my *= recip_norm;
        mz *= recip_norm;

        _2q0mx = 2.0f * q0 * mx;
        _2q0my = 2.0f * q0 * my;
        _2q0mz = 2.0f * q0 * mz;
        _2q1mx = 2.0f * q1 * mx;
        _2q0   = 2.0f * q0;
        _2q1   = 2.0f * q1;
        _2q2   = 2.0f * q2;
        _2q3   = 2.0f * q3;
        _2q0q2 = 2.0f * q0 * q2;
        _2q2q3 = 2.0f * q2 * q3;
        q0q0   = q0 * q0;
        q0q1   = q0 * q1;
        q0q2   = q0 * q2;
        q0q3   = q0 * q3;
        q1q1   = q1 * q1;
        q1q2   = q1 * q2;
        q1q3   = q1 * q3;
        q2q2   = q2 * q2;
        q2q3   = q2 * q3;
        q3q3   = q3 * q3;

        hx   = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
        hy   = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
        _2bx = sqrt(hx*hx + hy*hy);
        _2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
        _4bx = 2.0f * _2bx;
        _4bz = 2.0f * _2bz;

        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 =  _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 =  _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        
        recip_norm = invsqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        
        s0 *= recip_norm;
        s1 *= recip_norm;
        s2 *= recip_norm;
        s3 *= recip_norm;

        q_dot1 -= gain * s0;
        q_dot2 -= gain * s1;
        q_dot3 -= gain * s2;
        q_dot4 -= gain * s3;
    }

    q0 += q_dot1 * delay * 1e-3;
    q1 += q_dot2 * delay * 1e-3;
    q2 += q_dot3 * delay * 1e-3;
    q3 += q_dot4 * delay * 1e-3;

    recip_norm = invsqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    
    q0 *= recip_norm;
    q1 *= recip_norm;
    q2 *= recip_norm;
    q3 *= recip_norm;

    attitude->w = q0;
    attitude->i = q1;
    attitude->j = q2;
    attitude->k = q3;

    float accx;
    float accy;
    float accz;

    accx = 2*(q1*q3 - q0*q2);
    accy = 2*(q0*q1 + q2*q3);
    accz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

    gravity = accx*ax + accy*ay + accz*az;
}
