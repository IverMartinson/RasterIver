#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "math.h"

// ----- Meshs -----

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

typedef struct { // A loaded mesh file (NOT an actor; doesn't store transformations or textures, ONLY mesh file data)
    RI_face *faces;
    RI_vertex *vertecies;
    RI_vector_3f *normals;
    RI_vector_2f *uvs;
    int face_count;
    int vertex_count;
    int normal_count;
    int uv_count;
} RI_mesh; 

// ----- Textures -----
typedef struct { // A loaded texture file
    uint32_t *image_buffer;
    RI_vector_2 resolution;
} RI_texture;

typedef struct {
    char* filename;
    RI_vector_2 resolution; // The resolution the texture will be AFTER importing (only if you want to change an image's scale. Setting NULL is equal to providing the image's actual resolution)
} RI_texture_creation_data;

// ----- Actors -----
typedef struct { // An entity that has an mesh,
    RI_mesh *mesh_data;
} RI_actor;

typedef struct {
    RI_mesh *mesh_reference;
    RI_texture *texture_reference;
} RI_actor_creation_data;

#endif