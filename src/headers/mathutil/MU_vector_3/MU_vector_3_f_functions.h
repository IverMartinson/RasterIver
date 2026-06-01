#ifndef MU_V3F_H
#define MU_V3F_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec3u MU_3f_to_3u(MU_vec3i a){
    return (MU_vec3u){a.x, a.y, a.z};
}

MU_vec3i MU_3f_to_3i(MU_vec3f a){
    return (MU_vec3i){a.x, a.y, a.z};
}

MU_vec3d MU_3f_to_3d(MU_vec3d a){
    return (MU_vec3d){a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec3f MU_3f_add_3f(MU_vec3f a, MU_vec3f b){
    return (MU_vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_3f_add_3f_pointer(MU_vec3f *a, MU_vec3f *b){
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec3f MU_3f_sub_3f(MU_vec3f a, MU_vec3f b){
    return (MU_vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_3f_sub_3f_pointer(MU_vec3f *a, MU_vec3f *b){
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec3f MU_3f_mul_3f(MU_vec3f a, MU_vec3f b){
    return (MU_vec3f){a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_3f_mul_3f_pointer(MU_vec3f *a, MU_vec3f *b){
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec3f MU_3f_mul_f(MU_vec3f a, float scalar){
    return (MU_vec3f){a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec3f MU_f_mul_3f(float scalar, MU_vec3f a){
    return (MU_vec3f){a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_3f_mul_f_pointer(MU_vec3f *a, float scalar){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_f_mul_3f_pointer(float scalar, MU_vec3f *a){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec3f MU_3f_div_3f(MU_vec3f a, MU_vec3f b){
    return (MU_vec3f){a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_3f_div_3f_pointer(MU_vec3i *a, MU_vec3i *b){
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec3f MU_3f_div_f(MU_vec3f a, float scalar){
    return (MU_vec3f){a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec3f MU_f_div_3f(MU_vec3f a, float scalar){
    return (MU_vec3f){scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_3f_div_f_pointer(MU_vec3f *a, float scalar){
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_f_div_3f_pointer(float scalar, MU_vec3f *a){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions

double MU_3f_dot(MU_vec3f a, MU_vec3f b){
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

MU_vec3f MU_3f_cross(MU_vec3f a, MU_vec3f b){
    return (MU_vec3f){a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y};
}

double MU_3f_sqr_distance(MU_vec3f a, MU_vec3f b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_3f_distance(MU_vec3f a, MU_vec3f b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_3f_magnitude(MU_vec3f p){
    double magnitude = p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec3f MU_3f_normalize(MU_vec3f p){
    return MU_3f_div_f(p, MU_3f_magnitude(p));
}

#endif