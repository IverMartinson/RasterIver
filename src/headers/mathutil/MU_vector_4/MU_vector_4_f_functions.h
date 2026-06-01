#ifndef MU_V4F_H
#define MU_V4F_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec4u MU_4f_to_4u(MU_vec4i a){
    return (MU_vec4u){a.w, a.x, a.y, a.z};
}

MU_vec4i MU_4f_to_4i(MU_vec4f a){
    return (MU_vec4i){a.w, a.x, a.y, a.z};
}

MU_vec4d MU_4f_to_4d(MU_vec4d a){
    return (MU_vec4d){a.w, a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec4f MU_4f_add_4f(MU_vec4f a, MU_vec4f b){
    return (MU_vec4f){a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_4f_add_4f_pointer(MU_vec4f *a, MU_vec4f *b){
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec4f MU_4f_sub_4f(MU_vec4f a, MU_vec4f b){
    return (MU_vec4f){a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_4f_sub_4f_pointer(MU_vec4f *a, MU_vec4f *b){
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec4f MU_4f_mul_4f(MU_vec4f a, MU_vec4f b){
    return (MU_vec4f){a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_4f_mul_4f_pointer(MU_vec4f *a, MU_vec4f *b){
    a->w *= b->w;
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec4f MU_4f_mul_f(MU_vec4f a, float scalar){
    return (MU_vec4f){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec4f MU_f_mul_4f(float scalar, MU_vec4f a){
    return (MU_vec4f){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_4f_mul_f_pointer(MU_vec4f *a, float scalar){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_f_mul_4f_pointer(float scalar, MU_vec4f *a){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec4f MU_4f_div_4f(MU_vec4f a, MU_vec4f b){
    return (MU_vec4f){a.w / b.w, a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_4f_div_4f_pointer(MU_vec4f *a, MU_vec4f *b){
    a->w /= b->w;
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec4f MU_4f_div_f(MU_vec4f a, float scalar){
    return (MU_vec4f){a.w/scalar, a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec4f MU_f_div_4f(MU_vec4f a, float scalar){
    return (MU_vec4f){scalar/a.w, scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_4f_div_f_pointer(MU_vec4f *a, float scalar){
    a->w /= scalar;
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_f_div_4f_pointer(float scalar, MU_vec4f *a){
    a->w = scalar / a->w;
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions


double MU_4f_dot(MU_vec4f a, MU_vec4f b){
    return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_4f_sqr_distance(MU_vec4f a, MU_vec4f b){
    return pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_4f_distance(MU_vec4f a, MU_vec4f b){
    return sqrt(pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_4f_magnitude(MU_vec4f p){
    double magnitude = p.w*p.w + p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec4f MU_4f_normalize(MU_vec4f p){
    return MU_4f_div_f(p, MU_4f_magnitude(p));
}

#endif