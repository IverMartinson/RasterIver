#ifndef MU_V4D_H
#define MU_V4D_H

#include "../MU_types.h" 
#include <math.h>

// conversions

MU_vec4u MU_4d_to_4u(MU_vec4i a){
    return (MU_vec4u){a.w, a.x, a.y, a.z};
}

MU_vec4f MU_4d_to_4f(MU_vec4f a){
    return (MU_vec4f){a.w, a.x, a.y, a.z};
}

MU_vec4i MU_4d_to_4i(MU_vec4d a){
    return (MU_vec4i){a.w, a.x, a.y, a.z};
}

// arithmatic

// addition

MU_vec4d MU_4d_add_4d(MU_vec4d a, MU_vec4d b){
    return (MU_vec4d){a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
}

void MU_4d_add_4d_pointer(MU_vec4d *a, MU_vec4d *b){
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

// subtraction

MU_vec4d MU_4d_sub_4d(MU_vec4d a, MU_vec4d b){
    return (MU_vec4d){a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
}

void MU_4d_sub_4d_pointer(MU_vec4d *a, MU_vec4d *b){
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

// multiplication

MU_vec4d MU_4d_mul_4d(MU_vec4d a, MU_vec4d b){
    return (MU_vec4d){a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z};
}

void MU_4d_mul_4d_pointer(MU_vec4d *a, MU_vec4d *b){
    a->w *= b->w;
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

MU_vec4d MU_4d_mul_d(MU_vec4d a, double scalar){
    return (MU_vec4d){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

MU_vec4d MU_d_mul_4d(double scalar, MU_vec4d a){
    return (MU_vec4d){a.w*scalar, a.x*scalar, a.y*scalar, a.z*scalar};
}

void MU_4d_mul_d_pointer(MU_vec4d *a, double scalar){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

void MU_d_mul_4d_pointer(double scalar, MU_vec4d *a){
    a->w *= scalar;
    a->x *= scalar;
    a->y *= scalar;
    a->z *= scalar;
}

// division

MU_vec4d MU_4d_div_4d(MU_vec4d a, MU_vec4d b){
    return (MU_vec4d){a.w / b.w, a.x / b.x, a.y / b.y, a.z / b.z};
}

void MU_4d_div_4d_pointer(MU_vec4d *a, MU_vec4d *b){
    a->w /= b->w;
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
}

MU_vec4d MU_4d_div_d(MU_vec4d a, double scalar){
    return (MU_vec4d){a.w/scalar, a.x/scalar, a.y/scalar, a.z/scalar};
}

MU_vec4d MU_d_div_4d(MU_vec4d a, double scalar){
    return (MU_vec4d){scalar/a.w, scalar/a.x, scalar/a.y, scalar/a.z};
}

void MU_4d_div_d_pointer(MU_vec4d *a, double scalar){
    a->w /= scalar;
    a->x /= scalar;
    a->y /= scalar;
    a->z /= scalar;
}

void MU_d_div_4d_pointer(double scalar, MU_vec4d *a){
    a->w = scalar / a->w;
    a->x = scalar / a->x;
    a->y = scalar / a->y;
    a->z = scalar / a->z;
}

// vector specific functions


double MU_4d_dot(MU_vec4d a, MU_vec4d b){
    return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
}

double MU_4d_sqr_distance(MU_vec4d a, MU_vec4d b){
    return pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}

double MU_4d_distance(MU_vec4d a, MU_vec4d b){
    return sqrt(pow(a.w - b.w, 2) + pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double MU_4d_magnitude(MU_vec4d p){
    double magnitude = p.w*p.w + p.x*p.x + p.y*p.y + p.z*p.z;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec4d MU_4d_normalize(MU_vec4d p){
    return MU_4d_div_d(p, MU_4d_magnitude(p));
}

MU_quaternion MU_quaternion_mul_quaternion(MU_quaternion a, MU_quaternion b) {
    MU_quaternion product;
    product.w = (a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
    product.x = (a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y);
    product.y = (a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x);
    product.z = (a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w);
    return product;
}

MU_quaternion MU_quaternion_rotate(MU_vec3d axis, double angle, MU_quaternion quat){
    double divAngle = angle/2.0;
    double sinDivAngle = sin(divAngle);

    axis = MU_3d_normalize(axis);

    MU_quaternion local_rotation = {
        cos(divAngle),
        axis.x * sinDivAngle,
        axis.y * sinDivAngle,
        axis.z * sinDivAngle
    };

    return MU_4d_normalize(MU_quaternion_mul_quaternion(local_rotation, quat));
}

#endif