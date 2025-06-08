#ifndef OBJECT_H
#define OBJECT_H

#include "bugMath.h"
#include "stdint.h"

typedef struct {
    Vec3 position;
    Vec3 scale;
    Vec4 rotation;
} Transform;

typedef struct {
    int is_lit;
    uint32_t albedo;
    char texture[100];
    char normal_map[100];
    char height_map[100];
} Material;

typedef struct {
    char model_file[100];
    int polygon_count;
    int polygon_offset;
    int vertex_offset;
    int normal_offset;
    int uv_offset;
} Model;

typedef struct {
    Transform transform;
    Material material;
    Model model;
} Object;

#endif // OBJECT_H