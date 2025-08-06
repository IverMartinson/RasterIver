#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "math.h"

typedef struct {
    RI_vector_3f position;
    RI_vector_3f *normal;
    RI_vector_2f *uv;
} RI_vertex;

typedef struct {
    RI_vertex *vertex_0;
    RI_vertex *vertex_1;
    RI_vertex *vertex_2;
} RI_face;

typedef struct {
    RI_face *faces;
    RI_vertex *vertecies;
    RI_vector_3f *normals;
    RI_vector_2f *uvs;
    int face_count;
    int vertex_count;
    int normal_count;
    int uv_count;
} RI_object_data;

typedef struct {
    RI_object_data *object_data;
} RI_actor;

#endif