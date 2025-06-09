#include <math.h>

//SIXBUGSTHREEBRANCHES VERIFIED HEADER

#ifndef BUGMATH_H
#define BUGMATH_H


typedef struct {
    float x;
    float y;
} Vec2;

Vec2 vec2_add(Vec2 a, Vec2 b){
    Vec2 result = {a.x + b.x, a.y + b.y};
    
    return result;
}

Vec2 vec2_sub(Vec2 a, Vec2 b){
    Vec2 result = {a.x - b.x, a.y - b.y};
    
    return result;
}

Vec2 vec2_ham(Vec2 a, Vec2 b){
    Vec2 result = {a.x * b.x, a.y * b.y};
    
    return result;
}

Vec2 vec2_div(Vec2 a, Vec2 b){
    Vec2 result = {a.x / b.x, a.y / b.y};
    
    return result;
}

Vec2 vec2_scale_m(Vec2 v, float scalar){
    Vec2 result = {v.x*scalar, v.y*scalar};
    
    return result;
}

Vec2 vec2_scale_d(Vec2 v, float scalar){
    Vec2 result = {v.x/scalar, v.y/scalar};
    
    return result;
}

Vec2 vec2_perp(Vec2 v){
    Vec2 result = {-v.y, v.x};
    
    return result;
}

float vec2_mag(Vec2 v){
    float mag = v.x*v.x + v.y*v.y;
    if(mag == 1.0f){
        return mag;
    }

    return sqrtf(mag);
}

Vec2 vec2_norm(Vec2 v){
    return vec2_scale_d(v, vec2_mag(v));
}

float vec2_dot(Vec2 a, Vec2 b){
    return a.x*b.x + a.y*b.y;
}



typedef struct {
    float x;
    float y;
    float z;
} Vec3;

Vec3 vec3_add(Vec3 a, Vec3 b){
    Vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    
    return result;
}

Vec3 vec3_sub(Vec3 a, Vec3 b){
    Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    
    return result;
}

Vec3 vec3_ham(Vec3 a, Vec3 b){
    Vec3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
    
    return result;
}

Vec3 vec3_div(Vec3 a, Vec3 b){
    Vec3 result = {a.x / b.x, a.y / b.y, a.z / b.z};
 
    return result;
}

float vec3_dot(Vec3 a, Vec3 b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b){
    Vec3 result = {a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y};;
 
    return result;
}

Vec3 vec3_scale_m(Vec3 v, float scalar){
    Vec3 result = {v.x*scalar, v.y*scalar, v.z*scalar};
 
    return result;
}

Vec3 vec3_scale_d(Vec3 v, float scalar){
    Vec3 result = {v.x/scalar, v.y/scalar, v.z/scalar};
 
    return result;
}

float vec3_mag(Vec3 v){
    float mag = v.x*v.x + v.y*v.y + v.z*v.z;
    if(mag == 1.0f){
        return mag;
    }

    return sqrtf(mag);
}

Vec3 vec3_norm(Vec3 v){
    return vec3_scale_d(v, vec3_mag(v));
}




typedef struct {
    float w;
    float x;
    float y;
    float z;
} Vec4;

Vec4 vec4_add(Vec4 a, Vec4 b){
    Vec4 result = {a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
    
    return result;
}

Vec4 vec4_sub(Vec4 a, Vec4 b){
    Vec4 result = {a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
    
    return result;
}

Vec4 vec4_ham(Vec4 a, Vec4 b){
    Vec4 result = {a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z};

    return result;
}

Vec4 vec4_div(Vec4 a, Vec4 b){
    Vec4 result = {a.w / b.w, a.x / b.x, a.y / b.y, a.z / b.z};
    
    return result;
}

Vec4 quat_mul(Vec4 a, Vec4 b){
    Vec4 result;

    result.w = (a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
    result.x = (a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y);
    result.y = (a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x);
    result.z = (a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w);
    
    return result;
}

float vec4_dot(Vec4 a, Vec4 b){
    return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec4 vec4_scale_m(Vec4 q, float scalar){
    Vec4 result = {q.w*scalar, q.x*scalar, q.y*scalar, q.z*scalar};
    return result;
}

Vec4 vec4_scale_d(Vec4 q, float scalar){
    Vec4 result = {q.w/scalar, q.x/scalar, q.y/scalar, q.z/scalar};
    return result;
}

float vec4_mag(Vec4 q){
    float mag = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
    if(mag == 1.0f){
        return mag;
    }
    return sqrtf(mag);
}

Vec4 vec4_norm(Vec4 q){
    return vec4_scale_d(q, vec4_mag(q));
}




#endif
