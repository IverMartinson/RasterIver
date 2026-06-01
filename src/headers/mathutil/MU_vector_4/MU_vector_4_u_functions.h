#ifndef MU_V4U_H
#define MU_V4U_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec4i MU_4u_to_4i(MU_vec4i a){
    return (MU_vec4i){a.w, a.x, a.y, a.z};
}

MU_vec4f MU_4u_to_4f(MU_vec4f a){
    return (MU_vec4f){a.w, a.x, a.y, a.z};
}

MU_vec4d MU_4u_to_4d(MU_vec4d a){
    return (MU_vec4d){a.w, a.x, a.y, a.z};
}


// arithmatic

// addition

MU_vec4u MU_4u_add_4u(MU_vec4u a, MU_vec4u b){
    return (MU_vec4u){a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_4u_add_4u_pointer(MU_vec4u *a, MU_vec4u *b){
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec4u MU_4u_sub_4u(MU_vec4u a, MU_vec4u b){
    return (MU_vec4u){a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_4u_sub_4u_pointer(MU_vec4u *a, MU_vec4u *b){
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec4u MU_4u_mul_4u(MU_vec4u a, MU_vec4u b){
    return (MU_vec4u){a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_4u_mul_4u_pointer(MU_vec4u *a, MU_vec4u *b){
    a->w *= b->w;
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec4u MU_4u_mul_u(MU_vec4u a, u32 scalar){
    return (MU_vec4u){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec4u MU_u_mul_4u(u32 scalar, MU_vec4u a){
    return (MU_vec4u){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_4u_mul_u_pointer(MU_vec4u *a, u32 scalar){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_u_mul_4u_pointer(u32 scalar, MU_vec4u *a){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec4u MU_4u_div_4u(MU_vec4u a, MU_vec4u b){
    return (MU_vec4u){a.w / b.w, a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_4u_div_4u_pointer(MU_vec4u *a, MU_vec4u *b){
    a->w /= b->w;
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec4u MU_4u_div_u(MU_vec4u a, u32 scalar){
    return (MU_vec4u){a.w/scalar, a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec4u MU_u_div_4u(MU_vec4u a, u32 scalar){
    return (MU_vec4u){scalar/a.w, scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_4u_div_u_pointer(MU_vec4u *a, u32 scalar){
    a->w /= scalar;
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_u_div_4u_pointer(u32 scalar, MU_vec4u *a){
    a->w = scalar / a->w;
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions


double MU_4u_dot(MU_vec4u a, MU_vec4u b){
    return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_4u_sqr_distance(MU_vec4u a, MU_vec4u b){
    return pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_4u_distance(MU_vec4u a, MU_vec4u b){
    return sqrt(pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_4u_magnitude(MU_vec4u p){
    double magnitude = p.w*p.w + p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec4u MU_4u_normalize(MU_vec4u p){
    return MU_4u_div_u(p, MU_4u_magnitude(p));
}

#endif