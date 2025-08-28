#ifndef MATH_H
#define MATH_H

#include "stdint.h"
#include <math.h>

typedef struct {
    int x; 
    int y; 
} RI_vector_2;

typedef struct {
    double x; 
    double y; 
} RI_vector_2f;

typedef struct {
    double x; 
    double y; 
    double z; 
} RI_vector_3f;

typedef struct {
    int x; 
    int y; 
    int z; 
} RI_vector_3;

typedef struct {
    double w; 
    double x; 
    double y; 
    double z; 
} RI_vector_4f;

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_2f_times(RI_vector_2f *vector, double value){
    vector->x *= value;
    vector->y *= value;
}

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_2_times(RI_vector_2 *vector, double value){
    vector->x *= value;
    vector->y *= value;
}

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_3f_times(RI_vector_3f *vector, double value){
    vector->x *= value;
    vector->y *= value;
    vector->z *= value;
}

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_3_times(RI_vector_3 *vector, double value){
    vector->x *= value;
    vector->y *= value;
    vector->z *= value;
}

// hadamard multiplacation.
// multiply each value of one vector with the matching one on the other vector
void vector_3f_hadamard(RI_vector_3f *multiplicand, RI_vector_3f multiplicator){
    multiplicand->x *= multiplicator.x;
    multiplicand->y *= multiplicator.y;
    multiplicand->z *= multiplicator.z;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_2f_element_wise_add(RI_vector_2f *addend_a, RI_vector_2f addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_2_element_wise_add(RI_vector_2 *addend_a, RI_vector_2 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_3f_element_wise_add(RI_vector_3f *addend_a, RI_vector_3f addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_3_element_wise_add(RI_vector_3 *addend_a, RI_vector_3 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

// "hadamard" subtraction.
// subtraction each value of one vector with the matching one on the other vector
void vector_3f_element_wise_subtract(RI_vector_3f *minuend, RI_vector_3f subtrahend){
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
    minuend->z -= subtrahend.z;
}

// "hadamard" division.
// divide each value of one vector with the matching one on the other vector
void vector_3f_divide(RI_vector_3f *dividend, double divisor){
    dividend->x /= divisor;
    dividend->y /= divisor;
    dividend->z /= divisor;
}

// conjugate a quaterion.
// (flip the sign of the x, y, z values)
void quaternion_conjugate(RI_vector_4f* quaternion){
    quaternion->x *= -1.0;
    quaternion->y *= -1.0;
    quaternion->z *= -1.0;
}

// quaternion multiplacation
void quaternion_multiply(RI_vector_4f* a, RI_vector_4f b){
    double w1 = a->w; double x1 = a->x; double y1 = a->y; double z1 = a->z;
    double w2 = b.w; double x2 = b.x; double y2 = b.y; double z2 = b.z;

    double w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    double x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    double y = w1*y2 - x1*z2 + y1*w2 + z1*x2;
    double z = w1*z2 + x1*y2 - y1*x2 + z1*w2;

    *a = (RI_vector_4f){w, x, y, z};
}

// linear interpolate between 2 vectors
void vector_2f_lerp(RI_vector_2f vector_a, RI_vector_2f vector_b, RI_vector_2f *result, double w1){
    double w0 = 1.0 - w1;

    vector_2f_times(result, 0);

    vector_2f_times(&vector_a, w0);
    vector_2f_times(&vector_b, w1);

    vector_2f_element_wise_add(result, vector_a);
    vector_2f_element_wise_add(result, vector_b);
}

// linear interpolate between 2 vectors
void vector_2_lerp(RI_vector_2 vector_a, RI_vector_2 vector_b, RI_vector_2 *result, double w1){
    double w0 = 1.0 - w1;

    vector_2_times(result, 0);

    vector_2_times(&vector_a, w0);
    vector_2_times(&vector_b, w1);

    vector_2_element_wise_add(result, vector_a);
    vector_2_element_wise_add(result, vector_b);
}

// beziate between 2 vectors.
// A & C: end points of the line
// B: point that determines the curve; off the line
void vector_2f_bezier_interpolate(RI_vector_2f vector_a, RI_vector_2f vector_b, RI_vector_2f vector_c, RI_vector_2f *result, double w1){
    double w0 = 1.0 - w1;

    vector_2f_lerp(vector_a, vector_b, &vector_b, w1); // this works because the first vector b is a copy and the second is a reference
    vector_2f_lerp(vector_b, vector_c, &vector_c, w1);

    vector_2f_lerp(vector_b, vector_c, result, w1);
}

// beziate between 2 vectors.
// A & C: end points of the line
// B: point that determines the curve; off the line
void vector_2_bezier_interpolate(RI_vector_2 vector_a, RI_vector_2 vector_b, RI_vector_2 vector_c, RI_vector_2 *result, double w1){
    double w0 = 1.0 - w1;

    vector_2_lerp(vector_a, vector_b, &vector_b, w1); // this works because the first vector b is a copy and the second is a reference
    vector_2_lerp(vector_b, vector_c, &vector_c, w1);

    vector_2_lerp(vector_b, vector_c, result, w1);
}

// linear interpolate between 2 vectors
void vector_3f_lerp(RI_vector_3f vector_a, RI_vector_3f vector_b, RI_vector_3f *result, double w1){
    double w0 = 1.0 - w1;

    vector_3f_times(result, 0);

    vector_3f_times(&vector_a, w0);
    vector_3f_times(&vector_b, w1);

    vector_3f_element_wise_add(result, vector_a);
    vector_3f_element_wise_add(result, vector_b);
}

// linear interpolate between 2 vectors
void vector_3_lerp(RI_vector_3 vector_a, RI_vector_3 vector_b, RI_vector_3 *result, double w1){
    double w0 = 1.0 - w1;

    vector_3_times(result, 0);

    vector_3_times(&vector_a, w0);
    vector_3_times(&vector_b, w1);

    vector_3_element_wise_add(result, vector_a);
    vector_3_element_wise_add(result, vector_b);
}

// returns the distance between 2 points
int distance_2(RI_vector_2 a, RI_vector_2 b){
    return (int)sqrtf((float)((float)(a.x - b.x) * (float)(a.x - b.x)) + (float)((float)(a.y - b.y) * (float)(a.y - b.y)));
}

RI_vector_2f v2_to_2f(RI_vector_2 v){
    return (RI_vector_2f){v.x, v.y};
}

RI_vector_2 v2f_to_2(RI_vector_2f v){
    return (RI_vector_2){v.x, v.y};
}

void quaternion_rotate(RI_vector_3f *position, RI_vector_4f rotation){
    RI_vector_4f pos_quat = {0, position->x, position->y, position->z};

    RI_vector_4f rotation_conjugation = rotation;
    quaternion_conjugate(&rotation_conjugation);

    quaternion_multiply(&rotation, pos_quat);

    quaternion_multiply(&rotation, rotation_conjugation);

    *position = (RI_vector_3f){rotation.x, rotation.y, rotation.z};
}

void RI_euler_rotation_to_quaternion(RI_vector_4f *quaternion, RI_vector_3f euler_rotation){
    double cx = cosf(euler_rotation.x * 0.5f);
    double sx = sinf(euler_rotation.x * 0.5f);
    double cy = cosf(euler_rotation.y * 0.5f);
    double sy = sinf(euler_rotation.y * 0.5f);
    double cz = cosf(euler_rotation.z * 0.5f);
    double sz = sinf(euler_rotation.z * 0.5f);

    quaternion->w = cx * cy * cz + sx * sy * sz;
    quaternion->x = sx * cy * cz - cx * sy * sz;
    quaternion->y = cx * sy * cz + sx * cy * sz;
    quaternion->z = cx * cy * sz - sx * sy * cz;
}

#endif