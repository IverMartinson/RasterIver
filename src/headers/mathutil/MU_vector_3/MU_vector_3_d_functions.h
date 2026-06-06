#ifndef MU_V3D_H
#define MU_V3D_H

#include "../MU_types.h"
#include <math.h>

// conversions

MU_vec3u MU_3d_to_3u(MU_vec3i a){
    return (MU_vec3u){a.x, a.y, a.z};
}

MU_vec3f MU_3d_to_3f(MU_vec3f a){
    return (MU_vec3f){a.x, a.y, a.z};
}

MU_vec3i MU_3d_to_3i(MU_vec3d a){
    return (MU_vec3i){a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec3d MU_3d_add_3d(MU_vec3d a, MU_vec3d b){
    return (MU_vec3d){a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_3d_add_3d_pointer(MU_vec3d *a, MU_vec3d *b){
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec3d MU_3d_sub_3d(MU_vec3d a, MU_vec3d b){
    return (MU_vec3d){a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_3d_sub_3d_pointer(MU_vec3d *a, MU_vec3d *b){
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec3d MU_3d_mul_3d(MU_vec3d a, MU_vec3d b){
    return (MU_vec3d){a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_3d_mul_3d_pointer(MU_vec3d *a, MU_vec3d *b){
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec3d MU_3d_mul_d(MU_vec3d a, double scalar){
    return (MU_vec3d){a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec3d MU_d_mul_3d(double scalar, MU_vec3d a){
    return (MU_vec3d){a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_3d_mul_d_pointer(MU_vec3d *a, double scalar){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_d_mul_3d_pointer(double scalar, MU_vec3d *a){
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec3d MU_3d_div_3d(MU_vec3d a, MU_vec3d b){
    return (MU_vec3d){a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_3d_div_3d_pointer(MU_vec3d *a, MU_vec3d *b){
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec3d MU_3d_div_d(MU_vec3d a, double scalar){
    return (MU_vec3d){a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec3d MU_d_div_3d(MU_vec3d a, double scalar){
    return (MU_vec3d){scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_3d_div_d_pointer(MU_vec3d *a, double scalar){
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_d_div_3d_pointer(double scalar, MU_vec3d *a){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

MU_vec3d MU_vec4d_to_vec3d(MU_vec4d vector){
    return (MU_vec3d){vector.x, vector.y, vector.z};
}

// vector specific functions

MU_vec3d MU_vec3d_lerp(MU_vec3d a, MU_vec3d b, double t){
    double i = 1.0 - t;
    
    return (MU_vec3d){a.x * i + b.x * t, a.y * i + b.y * t, a.z * i + b.z * t};
}

double MU_3d_dot(MU_vec3d a, MU_vec3d b){
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_3d_sqr_distance(MU_vec3d a, MU_vec3d b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_3d_distance(MU_vec3d a, MU_vec3d b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_3d_magnitude(MU_vec3d p){
    double magnitude = p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec3d MU_3d_normalize(MU_vec3d p){
    return MU_3d_div_d(p, MU_3d_magnitude(p));
}

#endif