#ifndef RI_MATH_H
#define RI_MATH_H

#include "RI_int.h"
#include <math.h>

#ifndef RI_PI
#define RI_PI 3.14159265359;
#endif
#ifndef RI_PI_2
#define RI_PI_2 6.28318530718;
#endif

typedef struct {
    double x, y;
} RI_vec_2;

typedef struct {
    double x, y, z;
} RI_vec_3;

typedef struct {
    double w, x, y, z;
} RI_vec_4;

typedef RI_vec_4 RI_quaternion;

#endif