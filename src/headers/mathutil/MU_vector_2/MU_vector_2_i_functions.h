#ifndef MU_V2I_H
#define MU_V2I_H

#include "../MU_types.h"
#include <math.h>

// conversions

MU_vec2u MU_2i_to_2u(MU_vec2i a){
    return (MU_vec2u){a.x, a.y};
}

MU_vec2f MU_2i_to_2f(MU_vec2f a){
    return (MU_vec2f){a.x, a.y};
}

MU_vec2d MU_2i_to_2d(MU_vec2d a){
    return (MU_vec2d){a.x, a.y};
}

// arithmatic

// addition

MU_vec2i MU_2i_add_2i(MU_vec2i a, MU_vec2i b){
    return (MU_vec2i){a.x + b.x, a.y + b.y};
}

void MU_2i_add_2i_pointer(MU_vec2i *a, MU_vec2i *b){
    a->x += b->x;
    a->y += b->y;
}

// subtraction

MU_vec2i MU_2i_sub_2i(MU_vec2i a, MU_vec2i b){
    return (MU_vec2i){a.x - b.x, a.y - b.y};
}

void MU_2i_sub_2i_pointer(MU_vec2i *a, MU_vec2i *b){
    a->x -= b->x;
    a->y -= b->y;
}

// multiplication

MU_vec2i MU_2i_mul_2i(MU_vec2i a, MU_vec2i b){
    return (MU_vec2i){a.x * b.x, a.y * b.y};
}

void MU_2i_mul_2i_pointer(MU_vec2i *a, MU_vec2i *b){
    a->x *= b->x;
    a->y *= b->y;
}

MU_vec2i MU_2i_mul_i(MU_vec2i a, int scalar){
    return (MU_vec2i){a.x*scalar, a.y*scalar};
}

MU_vec2i MU_i_mul_2i(int scalar, MU_vec2i a){
    return (MU_vec2i){a.x*scalar, a.y*scalar};
}

void MU_2i_mul_i_pointer(MU_vec2i *a, int scalar){
    a->x *= scalar;
    a->y *= scalar;
}

void MU_i_mul_2i_pointer(int scalar, MU_vec2i *a){
    a->x *= scalar;
    a->y *= scalar;
}

// division

MU_vec2i MU_2i_div_2i(MU_vec2i a, MU_vec2i b){
    return (MU_vec2i){a.x / b.x, a.y / b.y};
}

void MU_2i_div_2i_pointer(MU_vec2i *a, MU_vec2i *b){
    a->x /= b->x;
    a->y /= b->y;
}

MU_vec2i MU_2i_div_i(MU_vec2i a, int scalar){
    return (MU_vec2i){a.x/scalar, a.y/scalar};
}

MU_vec2i MU_i_div_2i(MU_vec2i a, int scalar){
    return (MU_vec2i){scalar/a.x, scalar/a.y};
}

void MU_2i_div_i_pointer(MU_vec2i *a, int scalar){
    a->x /= scalar;
    a->y /= scalar;
}

void MU_i_div_2i_pointer(MU_vec2i *a, int scalar){
    a->x = scalar / a->x;
    a->y = scalar / a->y;
}

// vector specific functions

MU_vec2i MU_2i_perpendicular(MU_vec2i p){
    return (MU_vec2i){p.y, -p.x};
}

double MU_2i_dot(MU_vec2i a, MU_vec2i b){
    return (a.x*b.x + a.y*b.y);
}

double MU_2i_sqr_distance(MU_vec2i a, MU_vec2i b){
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
}

double MU_2i_distance(MU_vec2i a, MU_vec2i b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double MU_2i_magnitude(MU_vec2i p){
    double magnitude = p.x*p.x + p.y*p.y;
    
    if(magnitude == 1){
        return magnitude;
    }

    return sqrt(magnitude);
}

//not sure how useful normalizing integer vecs is, but it's consistant!!
MU_vec2i MU_2i_normalize(MU_vec2i p){
    return MU_2i_div_i(p, MU_2i_magnitude(p));
}


#endif