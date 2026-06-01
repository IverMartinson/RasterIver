#ifndef MU_V2U_H
#define MU_V2U_H

#include "../MU_types.h"
#include <math.h>

MU_vec2i MU_2u_to_2i(MU_vec2i a){
    return (MU_vec2i){a.x, a.y};
}

MU_vec2f MU_2u_to_2f(MU_vec2f a){
    return (MU_vec2f){a.x, a.y};
}

MU_vec2d MU_2u_to_2d(MU_vec2d a){
    return (MU_vec2d){a.x, a.y};
}

// arithmatic

// addition

MU_vec2u MU_2u_add_2u(MU_vec2u a, MU_vec2u b){
    return (MU_vec2u){a.x + b.x, a.y + b.y};
}

void MU_2u_add_2u_pointer(MU_vec2u *a, MU_vec2u *b){
    a->x += b->x;
    a->y += b->y;
}

// subtraction

MU_vec2u MU_2u_sub_2u(MU_vec2u a, MU_vec2u b){
    return (MU_vec2u){a.x - b.x, a.y - b.y};
}

void MU_2u_sub_2u_pointer(MU_vec2u *a, MU_vec2u *b){
    a->x -= b->x;
    a->y -= b->y;
}

// multiplication

MU_vec2u MU_2u_mul_2u(MU_vec2u a, MU_vec2u b){
    return (MU_vec2u){a.x * b.x, a.y * b.y};
}

void MU_2u_mul_2u_pointer(MU_vec2u *a, MU_vec2u *b){
    a->x *= b->x;
    a->y *= b->y;
}

MU_vec2u MU_2u_mul_i(MU_vec2u a, int scalar){
    return (MU_vec2u){a.x*scalar, a.y*scalar};
}

MU_vec2u MU_i_mul_2u(int scalar, MU_vec2u a){
    return (MU_vec2u){a.x*scalar, a.y*scalar};
}

void MU_2u_mul_u_pointer(MU_vec2u *a, int scalar){
    a->x*=scalar;
    a->y*=scalar;
}

void MU_u_mul_2u_pointer(int scalar, MU_vec2u *a){
    a->x*=scalar;
    a->y*=scalar;
}

// division

MU_vec2u MU_2u_div_2u(MU_vec2u a, MU_vec2u b){
    return (MU_vec2u){a.x / b.x, a.y / b.y};
}

void MU_2u_div_2u_pointer(MU_vec2u *a, MU_vec2u *b){
    a->x /= b->x;
    a->y /= b->y;
}


MU_vec2u MU_2u_div_u(MU_vec2u a, u32 scalar){
    return (MU_vec2u){a.x/scalar, a.y/scalar};
}

MU_vec2u MU_u_div_2u(MU_vec2u a, u32 scalar){
    return (MU_vec2u){scalar/a.x, scalar/a.y};
}

void MU_2u_div_u_pointer(MU_vec2u *a, u32 scalar){
    a->x /= scalar;
    a->y /= scalar;
}

void MU_u_div_2u_pointer(MU_vec2u *a, u32 scalar){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
}

// vector specific functions

//swaps x and y specifically for u32 vectors, since they can't be negative
MU_vec2u MU_2u_perpendicular(MU_vec2u p){
    return (MU_vec2u){p.y, p.x};
}

double MU_2u_dot(MU_vec2u a, MU_vec2u b){
    return (a.x*b.x + a.y*b.y);
}

double MU_2u_sqr_distance(MU_vec2u a, MU_vec2u b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
}

double MU_2u_distance(MU_vec2u a, MU_vec2u b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double MU_2u_magnitude(MU_vec2u p){
    double magnitude = p.x*p.x + p.y*p.y;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

MU_vec2u MU_2u_normalize(MU_vec2u p){
    return MU_2u_div_u(p, MU_2u_magnitude(p));
}

#endif