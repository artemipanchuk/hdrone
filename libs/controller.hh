#include <stdint.h>
#include <assert.h> 
#include <node.h> 

#include "ahrs.hh"
#include "pwm/afro_esc.hh"

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define PITCH_KP 2.0
#define PITCH_KI 5.0
#define PITCH_KD 0.5

#define ROLL_KP 2.0
#define ROLL_KI 5.0
#define ROLL_KD 0.5

#define ALT_KP 4.0
#define ALT_KI 5.0
#define ALT_KD 0.5

#define YAW_KP 3.0
#define YAW_KI 5.0
#define YAW_KD 0.5

#endif
