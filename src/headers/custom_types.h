#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "math.h"

// ----- Meshes -----
typedef struct {
    RI_vector_3f original_position;
    RI_vector_3f transformed_position;
    RI_vector_3f original_normal;
    RI_vector_3f transformed_normal;
    RI_vector_2f uv;
} RI_vertex;

typedef struct {
    RI_vertex *vertex_0;
    RI_vertex *vertex_1;
    RI_vertex *vertex_2;
} RI_face;

typedef struct { // A loaded mesh file (NOT an actor; doesn't store transformations or textures, ONLY mesh file data)
    RI_face *faces;
    RI_vertex *vertecies;
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
    RI_vector_2 resolution; // The resolution the texture will be AFTER importing (only if you want to change an image's scale. >=0 is equal to providing the image's actual resolution)
} RI_texture_creation_data;

// ----- Actors -----
typedef struct { 
    RI_vector_3f position;
    RI_vector_4f rotation;
    RI_vector_3f scale;
} RI_transform;

typedef struct {
    RI_texture *texture_reference;
    unsigned char albedo;
} RI_material;

typedef struct { // An entity that has an mesh, transform, materials, etc
    RI_mesh *mesh_reference;
    RI_material *material_reference;
    RI_transform transform;
} RI_actor;

typedef struct {
    RI_mesh *mesh_reference;
    RI_material *material_reference;
} RI_actor_creation_data;

// ----- Scene -----
typedef struct {
    RI_actor **actors;
    int actor_count;
} RI_scene;

#endif