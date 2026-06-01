#ifndef MU_V3I_H
#define MU_V3I_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec3u MU_3i_to_3u(MU_vec3i a){
    return (MU_vec3u){a.x, a.y, a.z};
}

MU_vec3f MU_3i_to_3f(MU_vec3f a){
    return (MU_vec3f){a.x, a.y, a.z};
}

MU_vec3d MU_3i_to_3d(MU_vec3d a){
    return (MU_vec3d){a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec3i MU_3i_add_3i(MU_vec3i a, MU_vec3i b){
    return (MU_vec3i){a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_3i_add_3i_pointer(MU_vec3i *a, MU_vec3i *b){
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec3i MU_3i_sub_3i(MU_vec3i a, MU_vec3i b){
    return (MU_vec3i){a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_3i_sub_3i_pointer(MU_vec3i *a, MU_vec3i *b){
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec3i MU_3i_mul_3i(MU_vec3i a, MU_vec3i b){
    return (MU_vec3i){a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_3i_mul_3i_pointer(MU_vec3i *a, MU_vec3i *b){
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec3i MU_3i_mul_i(MU_vec3i a, int scalar){
    return (MU_vec3i){a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec3i MU_i_mul_3i(int scalar, MU_vec3i a){
    return (MU_vec3i){a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_3i_mul_i_pointer(MU_vec3i *a, int scalar){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_i_mul_3i_pointer(int scalar, MU_vec3i *a){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec3i MU_3i_div_3i(MU_vec3i a, MU_vec3i b){
    return (MU_vec3i){a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_3i_div_3i_pointer(MU_vec3i *a, MU_vec3i *b){
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec3i MU_3i_div_i(MU_vec3i a, int scalar){
    return (MU_vec3i){a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec3i MU_i_div_3i(MU_vec3i a, int scalar){
    return (MU_vec3i){scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_3i_div_i_pointer(MU_vec3i *a, int scalar){
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_i_div_3i_pointer(int scalar, MU_vec3i *a){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions

double MU_3i_dot(MU_vec3i a, MU_vec3i b){
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_3i_sqr_distance(MU_vec3i a, MU_vec3i b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_3i_distance(MU_vec3i a, MU_vec3i b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_3i_magnitude(MU_vec3i p){
    double magnitude = p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec3i MU_3i_normalize(MU_vec3i p){
    return MU_3i_div_i(p, MU_3i_magnitude(p));
}

#endif