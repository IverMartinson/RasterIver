#include <math.h>

//SIXBUGSTHREEBRANCHES VERIFIED HEADER

#ifndef BUGMATH_H
#define BUGMATH_H


struct Vec2 {
    float x;
    float y;
};

struct Vec2 vec2_add(struct Vec2 a, struct Vec2 b){
    struct Vec2 result = {a.x + b.x, a.y + b.y};
    return result;
}

struct Vec2 vec2_sub(struct Vec2 a, struct Vec2 b){
    struct Vec2 result = {a.x - b.x, a.y - b.y};
    return result;
}

struct Vec2 vec2_mul(struct Vec2 a, struct Vec2 b){
    struct Vec2 result = {a.x * b.x, a.y * b.y};
    return result;
}

struct Vec2 vec2_div(struct Vec2 a, struct Vec2 b){
    struct Vec2 result = {a.x / b.x, a.y / b.y};
    return result;
}

struct Vec2 vec2_scale_m(struct Vec2 v, float scalar){
    struct Vec2 result = {v.x*scalar, v.y*scalar};
    return result;
}

struct Vec2 vec2_scale_d(struct Vec2 v, float scalar){
    struct Vec2 result = {v.x/scalar, v.y/scalar};
    return result;
}

struct Vec2 vec2_perp(struct Vec2 v){
    struct Vec2 result = {-v.y, v.x};
    return result;
}

float vec2_mag(struct Vec2 v){
    float mag = v.x*v.x + v.y*v.y;
    if(mag == 1.0f){
        return mag;
    }

    return sqrtf(mag);
}

struct Vec2 vec2_norm(struct Vec2 v){
    return vec2_scale_d(v, vec2_mag(v));
}

float vec2_dot(struct Vec2 a, struct Vec2 b){
    return a.x*b.x + a.y*b.y;
}



struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec3 vec3_add(struct Vec3 a, struct Vec3 b){
    struct Vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

struct Vec3 vec3_sub(struct Vec3 a, struct Vec3 b){
    struct Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

struct Vec3 vec3_mul(struct Vec3 a, struct Vec3 b){
    struct Vec3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
    return result;
}

struct Vec3 vec3_div(struct Vec3 a, struct Vec3 b){
    struct Vec3 result = {a.x / b.x, a.y / b.y, a.z / b.z};
    return result;
}

float vec3_dot(struct Vec3 a, struct Vec3 b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

struct Vec3 vec3_cross(struct Vec3 a, struct Vec3 b){
    struct Vec3 result = {a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y};;
    return result;
}

struct Vec3 vec3_scale_m(struct Vec3 v, float scalar){
    struct Vec3 result = {v.x*scalar, v.y*scalar, v.z*scalar};
    return result;
}

struct Vec3 vec3_scale_d(struct Vec3 v, float scalar){
    struct Vec3 result = {v.x/scalar, v.y/scalar, v.z/scalar};
    return result;
}

float vec3_mag(struct Vec3 v){
    float mag = v.x*v.x + v.y*v.y + v.z*v.z;
    if(mag == 1.0f){
        return mag;
    }

    return sqrtf(mag);
}

struct Vec3 vec3_norm(struct Vec3 v){
    return vec3_scale_d(v, vec3_mag(v));
}




struct Quaternion {
    float w;
    float x;
    float y;
    float z;
};

struct Quaternion quat_add(struct Quaternion a, struct Quaternion b){
    struct Quaternion result = {a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

struct Quaternion quat_sub(struct Quaternion a, struct Quaternion b){
    struct Quaternion result = {a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

struct Quaternion quat_mul(struct Quaternion a, struct Quaternion b){
    struct Quaternion result;

    result.w = (w*other.w - x*other.x - y*other.y - z*other.z);
    result.x = (w*other.x + x*other.w + y*other.z - z*other.y);
    result.y = (w*other.y - x*other.z + y*other.w + z*other.x);
    result.z = (w*other.z + x*other.y - y*other.x + z*other.w);
    
    return result;
}

float quat_dot(struct Quaternion a, struct Quaternion b){
    return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
}

struct Quaternion quat_scale_m(struct Quaternion q, float scalar){
    struct Quaternion result = {q.w*scalar, q.x*scalar, q.y*scalar, q.z*scalar};
    return result;
}

struct Quaternion quat_scale_d(struct Quaternion q, float scalar){
    struct Quaternion result = {q.w/scalar, q.x/scalar, q.y/scalar, q.z/scalar};
    return result;
}

float quat_mag(struct Quaternion q){
    float mag = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
    if(mag == 1.0f){
        return mag;
    }
    return sqrtf(mag);
}

struct Quaternion quat_norm(struct Quaternion q){
    return quat_scale_d(q, quat_mag(q));
}




#endif
