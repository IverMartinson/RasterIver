#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "math.h"
#include <stddef.h>

// ----- Meshes -----
typedef struct {
    int position_0_index;
    int position_1_index;
    int position_2_index;

    int normal_0_index;
    int normal_1_index;
    int normal_2_index;

    int uv_0_index;
    int uv_1_index;
    int uv_2_index;

    int should_render;
} RI_face;

typedef struct { // A loaded mesh file (NOT an actor; doesn't store transformations or textures, ONLY mesh file data)
    RI_face *faces;
    RI_vector_3f *vertex_positions;
    RI_vector_3f *normals;
    RI_vector_2f *uvs;
    int face_count, vertex_count, normal_count, uv_count;
    int has_normals, has_uvs;
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

typedef enum {
    RI_MATERIAL_UNLIT = ((uint64_t)1 << 0), // should calculate lighting
    RI_MATERIAL_DONT_CAST_SHADOW = ((uint64_t)1 << 1), // should cast shadow on other objects
    RI_MATERIAL_HAS_TEXTURE = ((uint64_t)1 << 2), // has a texture
    RI_MATERIAL_HAS_NORMAL_MAP = ((uint64_t)1 << 3), // has a normal map
    RI_MATERIAL_HAS_BUMP_MAP = ((uint64_t)1 << 4), // has a bump map
    RI_MATERIAL_TRANSPARENT = ((uint64_t)1 << 5), // has transparency
    RI_MATERIAL_WIREFRAME = ((uint64_t)1 << 6), // render as wireframe
    RI_MATERIAL_DONT_RECEIVE_SHADOW = ((uint64_t)1 << 7), // should shadows render on it
    RI_MATERIAL_DONT_DEPTH_TEST = ((uint64_t)1 << 8), // should check Z buffer (if 1, render on top of everything)
    RI_MATERIAL_DONT_DEPTH_WRITE = ((uint64_t)1 << 9), // should write to the Z buffer (if 1, render behind everything)
    RI_MATERIAL_DOUBLE_SIDED = ((uint64_t)1 << 10), // ignore backface culling
    RI_MATERIAL_USE_UV_LOOP_MULTIPLIER = ((uint64_t)1 << 11), // use multiplier that tells how many times to loop the texture (scaling also scales texture)
    RI_MATERIAL_USE_UV_RENDER_RESOLUTION = ((uint64_t)1 << 12), // use custom resolution that textures always appear as (scaling doesn't change texture)
    RI_MATERIAL_DONT_USE_AA = ((uint64_t)1 << 13), // disable anti-aliasing
} RI_material_flags;

typedef struct {
    RI_texture *texture_reference;
    RI_texture *bump_map_reference;
    RI_texture *normal_map_reference;
    unsigned int albedo;
    double wireframe_width;
    uint64_t flags;
    RI_vector_2f uv_loop_multiplier;
    RI_vector_2f texture_render_size;
} RI_material;

typedef struct { // An entity that has an mesh, transform, materials, etc
    RI_mesh *mesh_reference;
    RI_material *material_reference;
    RI_vector_3f *transformed_vertex_positions;
    RI_vector_3f *transformed_normals;
    RI_transform transform;
} RI_actor;

typedef struct {
    RI_mesh *mesh_reference;
    RI_material *material_reference;
} RI_actor_creation_data;

// ----- Scene -----
typedef struct {
    RI_vector_3f position_0, position_1, position_2;
    RI_vector_3f normal_0, normal_1, normal_2;
    RI_vector_2f uv_0, uv_1, uv_2;
    RI_material* material_reference;
    int min_screen_x, max_screen_x, min_screen_y, max_screen_y;
    int should_render;
    RI_actor* parent_actor;
    int shrunk;
    int split;
} RI_renderable_face;

typedef enum {
    RI_SCENE_UNLIT = ((uint64_t)1 << 0), // should calculate lighting
    RI_SCENE_WIREFRAME = ((uint64_t)1 << 1), // render as wireframe
    RI_SCENE_DOUBLE_SIDED = ((uint64_t)1 << 2), // ignore backface culling
    RI_SCENE_DONT_USE_AA = ((uint64_t)1 << 3), // disable anti-aliasing
    RI_SCENE_DEBUG_CULLS = ((uint64_t)1 << 4), // unchanged tris in grey, shrunk tris in blue, split triangles in green (old tri) and red (new tri)
    RI_SCENE_DEBUG_AABB = ((uint64_t)1 << 5), // AABB of polygons
    RI_SCENE_DEBUG_OVERDRAW = ((uint64_t)1 << 6), // overdraw
} RI_scene_flags;

typedef struct {
    RI_actor **actors;
    int actor_count;
    double FOV;
    double minimum_clip_distance;
    double min_clip;
    RI_vector_3f camera_position;
    RI_vector_4f camera_rotation;
    RI_renderable_face *faces_to_render;
    int face_count;
    int antialiasing_subsample_resolution;
    uint64_t flags;
} RI_scene;

// ----- Memory Manager -----
typedef struct {
    size_t size;
    void *pointer;
    int reallocated_free;
    int reallocated_alloc;
    int freed;
    int allocated;
    int line;
} RI_memory_allocation;

#endif