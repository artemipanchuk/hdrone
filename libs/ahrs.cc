#include "ahrs.hh"

float inv_sqrt(float x) {
    if (AHRS_INVSQRT_INSTABILITY_FIX == 0) {
        float halfx = 0.5f * x;
        float y = x;
        long  i = *(long*) &y;
        i = 0x5f3759df - (i >> 1);
        y = *(float*) &i;
        y = y * (1.5f - (halfx * y*y));
        
        return y;
    } else if (AHRS_INVSQRT_INSTABILITY_FIX == 1) {
        uint16_t i = 0x5F1F1412 - (*(unsigned int*)&x >> 1);
        float  tmp = *(float*) &i;

        return tmp * (1.69000231f - 0.714158168f * x * tmp*tmp);
    } else
        return 1.0f/sqrtf(x);
}

AHRS::AHRS(float gain, uint16_t delay) {
    this->quaternion[0] = 1;
    this->quaternion[1] = 0;
    this->quaternion[2] = 0;
    this->quaternion[3] = 0;

    if(gain < 0 || 1 < gain) {
        gain = 0.1;
        printf("[WARNING] [AHRS::AHRS] Invalid gain. Ignoring. Set to 0.1.\n");
    }

    this->gain = gain;

    if(delay < 0 || 1000 < delay) {
        delay = 100;
        printf("[WARNING] [AHRS::AHRS] Invalid frequency. Ignoring. Set to 100 ms.\n");
    }

    this->delay = delay;
}

void AHRS::update(float gx, float gy, float gz,
                  float ax, float ay, float az,
                  float mx, float my, float mz) {
    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot1, q_dot2, q_dot3, q_dot4;
    float hx, hy;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx,
          _2bx, _2bz, _4bx, _4bz,
          _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3,
          q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    uint16_t delay = this->delay;
    float gain     = this->gain;
    float q0 = this->quaternion[0],
          q1 = this->quaternion[1],
          q2 = this->quaternion[2],
          q3 = this->quaternion[3];

    q_dot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    q_dot2 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    q_dot3 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    q_dot4 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        recip_norm = inv_sqrt(mx * mx + my * my + mz * mz);
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
        
        recip_norm = inv_sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        
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

    recip_norm = inv_sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    
    q0 *= recip_norm;
    q1 *= recip_norm;
    q2 *= recip_norm;
    q3 *= recip_norm;

    this->quaternion[0] = q0;
    this->quaternion[1] = q1;
    this->quaternion[2] = q2;
    this->quaternion[3] = q3;
}
