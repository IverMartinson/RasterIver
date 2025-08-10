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

void vector_2f_times_float(RI_vector_2f *vector, float value){
    vector->x *= value;
    vector->y *= value;
}

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

void vector_2f_element_wise_add(RI_vector_2f *addend_a, RI_vector_2f addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
}

void vector_3f_element_wise_add(RI_vector_3f *addend_a, RI_vector_3f addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

void vector_3f_element_wise_subtract(RI_vector_3f *minuend, RI_vector_3f subtrahend){
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
    minuend->z -= subtrahend.z;
}

void vector_3f_divide_float(RI_vector_3f *dividend, float divisor){
    dividend->x /= divisor;
    dividend->y /= divisor;
    dividend->z /= divisor;
}

void quaternion_conjugate(RI_vector_4f* quaternion){
    quaternion->x *= -1;
    quaternion->y *= -1;
    quaternion->z *= -1;
}

void quaternion_multiply(RI_vector_4f* a, RI_vector_4f b){
    float w1 = a->w; float x1 = a->x; float y1 = a->y; float z1 = a->z;
    float w2 = b.w; float x2 = b.x; float y2 = b.y; float z2 = b.z;

    float w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    float x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    float y = w1*y2 - x1*z2 + y1*w2 + z1*x2;
    float z = w1*z2 + x1*y2 - y1*x2 + z1*w2;

    *a = (RI_vector_4f){w, x, y, z};
}

void vector_2f_lerp(RI_vector_2f vector_a, RI_vector_2f vector_b, RI_vector_2f *result, float w1){
    float w0 = 1.0 - w1;

    vector_2f_times_float(result, 0);

    vector_2f_times_float(&vector_a, w0);
    vector_2f_times_float(&vector_b, w1);

    vector_2f_element_wise_add(result, vector_a);
    vector_2f_element_wise_add(result, vector_b);
}

void vector_3f_lerp(RI_vector_3f vector_a, RI_vector_3f vector_b, RI_vector_3f *result, float w1){
    float w0 = 1.0 - w1;

    vector_3f_times_float(result, 0);

    vector_3f_times_float(&vector_a, w0);
    vector_3f_times_float(&vector_b, w1);

    vector_3f_element_wise_add(result, vector_a);
    vector_3f_element_wise_add(result, vector_b);
}


#endif