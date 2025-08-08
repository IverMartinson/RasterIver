#ifndef MATH_H
#define MATH_H

#include "stdint.h"

typedef struct {
    int x; 
    int y; 
} RI_vector_2;

typedef struct {
    float x; 
    float y; 
} RI_vector_2f;

typedef struct {
    float x; 
    float y; 
    float z; 
} RI_vector_3f;

typedef struct {
    float w; 
    float x; 
    float y; 
    float z; 
} RI_vector_4f;

void vector_3f_times_float(RI_vector_3f *vector, float value){
    vector->x *= value;
    vector->y *= value;
    vector->z *= value;
}

void vector_3f_hadamard(RI_vector_3f *multiplicand, RI_vector_3f multiplicator){
    multiplicand->x *= multiplicator.x;
    multiplicand->y *= multiplicator.y;
    multiplicand->z *= multiplicator.z;
}

void vector_3f_element_wise_add(RI_vector_3f *addend_a, RI_vector_3f addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

void vector_3f_divide_float(RI_vector_3f *dividend, float divisor){
    dividend->x /= divisor;
    dividend->y /= divisor;
    dividend->z /= divisor;
}

#endif