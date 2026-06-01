#ifndef MU_V2F_H
#define MU_V2F_H

#include "../MU_types.h"
#include <math.h>

// conversion

MU_vec2u MU_2f_to_2u(MU_vec2i a){
    return (MU_vec2u){a.x, a.y};
}

MU_vec2i MU_2f_to_2i(MU_vec2f a){
    return (MU_vec2i){a.x, a.y};
}

MU_vec2d MU_2f_to_2d(MU_vec2d a){
    return (MU_vec2d){a.x, a.y};
}

// artihmatic

// addition

MU_vec2f MU_2f_add_2f(MU_vec2f a, MU_vec2f b){
    return (MU_vec2f){a.x + b.x, a.y + b.y};
}

void MU_2f_add_2f_pointer(MU_vec2f *a, MU_vec2f *b){
    a->x += b->x;
    a->y += b->y;
}

// subtraction

MU_vec2f MU_2f_sub_2f(MU_vec2f a, MU_vec2f b){
    return (MU_vec2f){a.x - b.x, a.y - b.y};
}

void MU_2f_sub_2f_pointer(MU_vec2f *a, MU_vec2f *b){
    a->x -= b->x;
    a->y -= b->y;
}

// multiplication

MU_vec2f MU_2f_mul_2f(MU_vec2f a, MU_vec2f b){
    return (MU_vec2f){a.x * b.x, a.y * b.y};
}

void MU_2f_mul_2f_pointer(MU_vec2f *a, MU_vec2f *b){
    a->x *= b->x;
    a->y *= b->y;
}

MU_vec2f MU_2f_mul_i(MU_vec2f a, int scalar){
    return (MU_vec2f){a.x*scalar, a.y*scalar};
}

MU_vec2f MU_i_mul_2f(int scalar, MU_vec2f a){
    return (MU_vec2f){a.x*scalar, a.y*scalar};
}

void MU_2f_mul_i_pointer(MU_vec2f *a, int scalar){
    a->x*=scalar;
    a->y*=scalar;
}

void MU_i_mul_2f_pointer(int scalar, MU_vec2f *a){
    a->x*=scalar;
    a->y*=scalar;
}

// division

MU_vec2f MU_2f_div_2f(MU_vec2f a, MU_vec2f b){
    return (MU_vec2f){a.x / b.x, a.y / b.y};
}

void MU_2f_div_2f_pointer(MU_vec2f *a, MU_vec2f *b){
    a->x /= b->x;
    a->y /= b->y;
}

MU_vec2f MU_2f_div_f(MU_vec2f a, float scalar){
    return (MU_vec2f){a.x/scalar, a.y/scalar};
}

MU_vec2f MU_f_div_2f(MU_vec2f a, float scalar){
    return (MU_vec2f){scalar/a.x, scalar/a.y};
}

void MU_2f_div_f_pointer(MU_vec2i *a, float scalar){
    a->x /= scalar;
    a->y /= scalar;
}

void MU_f_div_2f_pointer(MU_vec2i *a, float scalar){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
}

// vector specific functions

MU_vec2f MU_2f_perpendicular(MU_vec2f p){
    return (MU_vec2f){p.y, -p.x};
}

double MU_2f_dot(MU_vec2f a, MU_vec2f b){
    return (a.x*b.x + a.y*b.y);
}

double MU_2f_sqr_distance(MU_vec2f a, MU_vec2f b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
}

double MU_2f_distance(MU_vec2f a, MU_vec2f b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double MU_2f_magnitude(MU_vec2f p){
    double magnitude = p.x*p.x + p.y*p.y;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec2f MU_2f_normalize(MU_vec2f p){
    return MU_2f_div_f(p, MU_2f_magnitude(p));
}

#endif