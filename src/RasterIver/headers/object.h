#ifndef OBJECT_H
#define OBJECT_H

#include <CL/cl_platform.h>

typedef struct __attribute__((aligned(4))) {
    cl_uchar a; // 1
    cl_uchar r; // 1
    cl_uchar g; // 1
    cl_uchar b; // 1
} ColorARGB; // size: 4, align: 4

typedef struct __attribute__((aligned(8))) {
    ColorARGB albedo;     // 4
    cl_int textureOffset; // 4
} Material; // size 8, align: 4

typedef struct __attribute__((aligned(16))) {
    cl_float x; // 4
    cl_float y; // 4
    cl_float z; // 4
    cl_float _pad0; // padding to align to 16 bytes
} Vec3;

typedef struct __attribute__((aligned(16))) {
    cl_float w;
    cl_float x;
    cl_float y;
    cl_float z;
} Vec4;

typedef struct __attribute__((aligned(16))) {
    Vec3 position;  // 16 bytes
    Vec3 scale;     // 16 bytes
    Vec4 rotation;  // 16 bytes
} Transform; // size: 48 bytes (actually 64 because 3x16)

typedef struct __attribute__((aligned(4))) {
    cl_int transformedVertexOffset;
    cl_int transformedNormalOffset;
    cl_int triangleCount;
    cl_int vertexCount;
    cl_int normalCount;
    cl_int uvCount;
    cl_int triangleOffset;
    cl_int vertexOffset;
    cl_int normalOffset;
    cl_int uvOffset;
} ModelInfo; // 10 × 4 = 40 bytes

typedef struct __attribute__((aligned(16))) {
    Transform transform;   // 64
    ModelInfo modelInfo;   // 40
    cl_int id;             // 4
    cl_int _pad1;          // 4 (pad so Material starts at 8-byte boundary)
    Material material;     // 8
    cl_int _pad2;          // 4 (pad to make Object size 128 bytes)
    cl_int _pad3;          // 4
} Object; // total: 64 + 40 + 4 + 4 + 8 + 4 + 4 = 128 bytes

#endif