#ifndef MU_TYPES_H
#define MU_TYPES_H

#include <stdint.h>

typedef int8_t i8; // -128 to 127
typedef uint8_t u8; // 0 to 255
typedef int16_t i16; // -32,768 to 32,767
typedef uint16_t u16; // 0 to 65535
typedef int32_t i32; // -2,147,483,648 to 2,147,483,647
typedef uint32_t u32; // 0 to 4,294,967,295
typedef int64_t i64; // -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 (quintillion)
typedef uint64_t u64; // 0 to 18,446,744,073,709,551,615 (quintillion)

// vector 2

typedef struct {
    i32 x, y;
} MU_vec2i;

typedef struct {
    u32 x, y;
} MU_vec2u;

typedef struct {
    float x, y;
} MU_vec2f;

typedef MU_vec2f MU_vec2;

typedef struct {
    double x, y;
} MU_vec2d;

// vector 3

typedef struct {
    i32 x, y, z;
} MU_vec3i;

typedef struct {
    u32 x, y, z;
} MU_vec3u;

typedef struct {
    float x, y, z;
} MU_vec3f;

typedef MU_vec3f MU_vec3;

typedef struct {
    double x, y, z;
} MU_vec3d;

// vector 4

typedef struct {
    i32 w, x, y, z;
} MU_vec4i;

typedef struct {
    u32 w, x, y, z;
} MU_vec4u;

typedef struct {
    float w, x, y, z;
} MU_vec4f;

typedef MU_vec4f MU_vec4;

typedef struct {
    double w, x, y, z;
} MU_vec4d;

typedef MU_vec4d MU_quaternion;

#endif