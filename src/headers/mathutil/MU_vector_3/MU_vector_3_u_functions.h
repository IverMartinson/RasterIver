#ifndef MU_V3U_H
#define MU_V3U_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec3i MU_3u_to_3i(MU_vec3i a){
    return (MU_vec3i){a.x, a.y, a.z};
}

MU_vec3f MU_3u_to_3f(MU_vec3f a){
    return (MU_vec3f){a.x, a.y, a.z};
}

MU_vec3d MU_3u_to_3d(MU_vec3d a){
    return (MU_vec3d){a.x, a.y, a.z};
}

MU_vec3u MU_3u_add_3u(MU_vec3u a, MU_vec3u b){
    return (MU_vec3u){a.x + b.x, a.y + b.y, a.z + b.z};
}

// arithmatic

// addition

void MU_3u_add_3u_pointer(MU_vec3u *a, MU_vec3u *b){
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec3u MU_3u_sub_3u(MU_vec3u a, MU_vec3u b){
    return (MU_vec3u){a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_3u_sub_3u_pointer(MU_vec3u *a, MU_vec3u *b){
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec3u MU_3u_mul_3u(MU_vec3u a, MU_vec3u b){
    return (MU_vec3u){a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_3u_mul_3u_pointer(MU_vec3u *a, MU_vec3u *b){
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec3u MU_3u_mul_u(MU_vec3u a, u32 scalar){
    return (MU_vec3u){a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec3u MU_u_mul_3i(u32 scalar, MU_vec3u a){
    return (MU_vec3u){a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_3u_mul_u_pointer(MU_vec3u *a, u32 scalar){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_u_mul_3u_pointer(u32 scalar, MU_vec3u *a){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec3u MU_3u_div_3u(MU_vec3u a, MU_vec3u b){
    return (MU_vec3u){a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_3u_div_3u_pointer(MU_vec3u *a, MU_vec3u *b){
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec3u MU_3u_div_u(MU_vec3u a, u32 scalar){
    return (MU_vec3u){a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec3u MU_u_div_3u(MU_vec3u a, u32 scalar){
    return (MU_vec3u){scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_3u_div_u_pointer(MU_vec3u *a, u32 scalar){
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_u_div_3u_pointer(u32 scalar, MU_vec3u *a){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions

double MU_3u_dot(MU_vec3u a, MU_vec3u b){
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_3u_sqr_distance(MU_vec3u a, MU_vec3u b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_3u_distance(MU_vec3u a, MU_vec3u b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_3u_magnitude(MU_vec3u p){
    double magnitude = p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec3u MU_3u_normalize(MU_vec3u p){
    return MU_3u_div_u(p, MU_3u_magnitude(p));
}

#endif