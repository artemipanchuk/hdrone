#include "quaternion.hh"

float invsqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long  i = *(long*) &y;
    
    i = 0x5f3759df - (i >> 1);
    y = *(float*) &i;
    y = y * (1.5f - (halfx * y*y));
    
    return y;
}

Quaternion::Quaternion(void) {
    w = 1.0;
    i = 0.0;
    j = 0.0;
    k = 0.0;
}

Quaternion::Quaternion(Quaternion *source) {
    w = source->w;
    i = source->i;
    j = source->j;
    k = source->k;
}

void Quaternion::invert(void) {
    i *= -1;
    j *= -1;
    k *= -1;
}

void Quaternion::normalize(void) {
    float n = invsqrt(w*w + i*i + j*j + k*k);

    w *= n;
    i *= n;
    j *= n;
    k *= n;
}

float Quaternion::pitch(void) {
    return asin(2*(w*j - i*k));
}

float Quaternion::roll(void) {
    return rot_x();
}

float Quaternion::yaw(void) {
    return rot_z();
}

float Quaternion::rot_x(void) {
    return atan2(2*(w*j + i*k), 1 - 2*(i*i + j*j));
}

float Quaternion::rot_y(void) {
    return atan2(2*(w*i + j*k), 1 - 2*(i*i + j*j));
}

float Quaternion::rot_z(void) {
    return atan2(2*(w*k + i*j), 1 - 2*(j*j + k*k));
}

void Quaternion::multiply(Quaternion* other) {
    float pw, pi, pj, pk;

    pw = w*other->w - i*other->i - j*other->j - k*other->k;
    pi = w*other->i + i*other->w + j*other->k - k*other->j;
    pj = w*other->j - i*other->k + j*other->w + k*other->i;
    pk = w*other->k + i*other->j - j*other->i + k*other->w;

    w = pw;
    i = pi;
    j = pj;
    k = pk;
}
