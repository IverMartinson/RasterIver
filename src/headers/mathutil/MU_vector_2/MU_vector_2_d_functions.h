#ifndef MU_V2D_H
#define MU_V2D_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec2u MU_2d_to_2u(MU_vec2i a){
    return (MU_vec2u){a.x, a.y};
}

MU_vec2f MU_2d_to_2f(MU_vec2f a){
    return (MU_vec2f){a.x, a.y};
}

MU_vec2i MU_2d_to_2i(MU_vec2d a){
    return (MU_vec2i){a.x, a.y};
}

// arithmatic

// addition

MU_vec2d MU_2d_add_2d(MU_vec2d a, MU_vec2d b){
    return (MU_vec2d){a.x + b.x, a.y + b.y};
}

void MU_2d_add_2d_pointer(MU_vec2d *a, MU_vec2d *b){
    a->x += b->x;
    a->y += b->y;
}

// subtraction

MU_vec2d MU_2d_sub_2d(MU_vec2d a, MU_vec2d b){
    return (MU_vec2d){a.x - b.x, a.y - b.y};
}

void MU_2d_sub_2d_pointer(MU_vec2d *a, MU_vec2d *b){
    a->x -= b->x;
    a->y -= b->y;
}

// multiplication

MU_vec2d MU_2d_mul_2d(MU_vec2d a, MU_vec2d b){
    return (MU_vec2d){a.x * b.x, a.y * b.y};
}

void MU_2d_mul_2d_pointer(MU_vec2d *a, MU_vec2d *b){
    a->x *= b->x;
    a->y *= b->y;
}

MU_vec2d MU_2d_mul_i(MU_vec2d a, int scalar){
    return (MU_vec2d){a.x*scalar, a.y*scalar};
}

MU_vec2d MU_i_mul_2d(int scalar, MU_vec2d a){
    return (MU_vec2d){a.x*scalar, a.y*scalar};
}

void MU_2d_mul_i_pointer(MU_vec2d *a, int scalar){
    a->x*=scalar;
    a->y*=scalar;
}

void MU_i_mul_2d_pointer(int scalar, MU_vec2d *a){
    a->x*=scalar;
    a->y*=scalar;
}

// division

MU_vec2d MU_2d_div_2d(MU_vec2d a, MU_vec2d b){
    return (MU_vec2d){a.x / b.x, a.y / b.y};
}

void MU_2d_div_2d_pointer(MU_vec2d *a, MU_vec2d *b){
    a->x /= b->x;
    a->y /= b->y;
}

MU_vec2d MU_2d_div_d(MU_vec2d a, double scalar){
    return (MU_vec2d){a.x/scalar, a.y/scalar};
}

MU_vec2d MU_f_div_2d(MU_vec2d a, double scalar){
    return (MU_vec2d){scalar/a.x, scalar/a.y};
}

void MU_2d_div_d_pointer(MU_vec2i *a, double scalar){
    a->x /= scalar;
    a->y /= scalar;
}

void MU_d_div_2d_pointer(MU_vec2i *a, double scalar){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
}

// vector specific functions

MU_vec2d MU_2d_perpendicular(MU_vec2d p){
    return (MU_vec2d){-p.y, p.x};
}

double MU_2d_dot(MU_vec2d a, MU_vec2d b){
    return (a.x*b.x + a.y*b.y);
}

double MU_2d_sqr_distance(MU_vec2d a, MU_vec2d b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
}

double MU_2d_distance(MU_vec2d a, MU_vec2d b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double MU_2d_magnitude(MU_vec2d p){
    double magnitude = p.x*p.x + p.y*p.y;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec2d MU_2d_normalize(MU_vec2d p){
    return MU_2d_div_d(p, MU_2d_magnitude(p));
}

#endif