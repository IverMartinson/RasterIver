#ifndef OBJECT_H
#define OBJECT_H

#include <CL/cl_platform.h>

typedef struct {
    float x;
    float y;
} Vec2;

// linealerly interpolates from point 1 to point 2
// if fraction is 0, it returns point 1
Vec3 lerp3(Vec3 pos1, Vec3 pos2, float fraction){
    Vec3 result;

    float w0 = 1.0 - fraction;
    float w1 = fraction;

    result.x = pos1.x * w0 + pos2.x * w1;
    result.y = pos1.y * w0 + pos2.y * w1;
    result.z = pos1.z * w0 + pos2.z * w1;

    return result;

}

typedef struct __attribute__((aligned(4))) {
    cl_uchar a; // 1
    cl_uchar r; // 1
    cl_uchar g; // 1
    cl_uchar b; // 1
} ColorARGB; // size: 4, align: 4

typedef struct __attribute__((aligned(4))) {
    ColorARGB albedo;     // 4
    cl_int textureOffset; // 4
    cl_int normalMapOffset; // 4
    cl_int bumpMapOffset; // 4
    cl_ulong properties; // 8
} Material; // size 24

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
} Transform; // size: 48 bytes

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
    cl_int split_triangles;          // 4 (pad so Material starts at 8-byte boundary)
    Material material;     // 8
    cl_int _pad2;          // 4 (pad to make Object size 128 bytes)
    cl_int _pad3;          // 4
} Object; // total: 64 + 40 + 4 + 4 + 8 + 4 + 4 = 128 bytes



typedef struct {
    Vec3 point0;
    Vec3 point1;
    Vec3 point2;
    cl_float frac_0;
    cl_float frac_1;
    cl_float frac_2;
    cl_int should_render;
} Triangle;

typedef struct {
    cl_int vertex0;
    cl_int vertex1;
    cl_int vertex2;
    cl_int uv0;
    cl_int uv1;
    cl_int uv2;
    cl_int normal0;
    cl_int normal1;
    cl_int normal2;
} TriangleData;

#endif