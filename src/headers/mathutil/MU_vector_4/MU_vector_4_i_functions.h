#ifndef MU_V4I_H
#define MU_V4I_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec4u MU_4i_to_4u(MU_vec4i a){
    return (MU_vec4u){a.w, a.x, a.y, a.z};
}

MU_vec4f MU_4i_to_4f(MU_vec4f a){
    return (MU_vec4f){a.w, a.x, a.y, a.z};
}

MU_vec4d MU_4i_to_4d(MU_vec4d a){
    return (MU_vec4d){a.w, a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec4i MU_4i_add_4i(MU_vec4i a, MU_vec4i b){
    return (MU_vec4i){a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_4i_add_4i_pointer(MU_vec4i *a, MU_vec4i *b){
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec4i MU_4i_sub_4i(MU_vec4i a, MU_vec4i b){
    return (MU_vec4i){a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_4i_sub_4i_pointer(MU_vec4i *a, MU_vec4i *b){
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec4i MU_4i_mul_4i(MU_vec4i a, MU_vec4i b){
    return (MU_vec4i){a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_4i_mul_4i_pointer(MU_vec4i *a, MU_vec4i *b){
    a->w *= b->w;
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec4i MU_4i_mul_i(MU_vec4i a, int scalar){
    return (MU_vec4i){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec4i MU_i_mul_4i(int scalar, MU_vec4i a){
    return (MU_vec4i){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_4i_mul_i_pointer(MU_vec4i *a, int scalar){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_i_mul_4i_pointer(int scalar, MU_vec4i *a){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec4i MU_4i_div_4i(MU_vec4i a, MU_vec4i b){
    return (MU_vec4i){a.w / b.w, a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_4i_div_4i_pointer(MU_vec4i *a, MU_vec4i *b){
    a->w /= b->w;
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec4i MU_4i_div_i(MU_vec4i a, int scalar){
    return (MU_vec4i){a.w/scalar, a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec4i MU_i_div_4i(MU_vec4i a, int scalar){
    return (MU_vec4i){scalar/a.w, scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_4i_div_i_pointer(MU_vec4i *a, int scalar){
    a->w /= scalar;
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_i_div_4i_pointer(int scalar, MU_vec4i *a){
    a->w = scalar / a->w;
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions


double MU_4i_dot(MU_vec4i a, MU_vec4i b){
    return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_4i_sqr_distance(MU_vec4i a, MU_vec4i b){
    return pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_4i_distance(MU_vec4i a, MU_vec4i b){
    return sqrt(pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_4i_magnitude(MU_vec4i p){
    double magnitude = p.w*p.w + p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec4i MU_4i_normalize(MU_vec4i p){
    return MU_4i_div_i(p, MU_4i_magnitude(p));
}

#endif