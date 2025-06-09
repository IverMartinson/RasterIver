#ifndef OBJECT_H
#define OBJECT_H

#include "bugMath.h"

struct ColorARGB {
    char a;
    char r;
    char g;
    char b;
};

struct Material {
    ColorARGB albedo;
    int textureOffset;
};

struct Transform {
    Vec3 position;
    Vec3 scale;
    Vec4 rotation;
};

struct ModelInfo {
    int triangleCount;
    int triangleOffset;
    int vertexOffset;
    int normalOffset;
    int uvOffset;
};

struct Object {
    Transform transform;
    Material material;
    ModelInfo modelInfo;
    int id;
};

#endif