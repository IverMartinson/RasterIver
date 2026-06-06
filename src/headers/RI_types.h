#ifndef RI_STRUCTS_H
#define RI_STRUCTS_H

#include <SDL2/SDL.h>
#include "mathutil.h"
#include "pitmap.h"

typedef MU_vec3d RI_position;
typedef MU_vec3d RI_scale;
typedef MU_vec4d RI_quaternion;

typedef MU_vec3d RI_vertex;
typedef MU_vec3d RI_normal;
typedef MU_vec2d RI_uv;

typedef struct {
    u32 v0, v1, v2;
    u32 n0, n1, n2;
    u32 u0, u1, u2;
} RI_triangle;

typedef struct {
    MU_vec3d v0, v1, v2;
    MU_vec3d n0, n1, n2;
    MU_vec2d u0, u1, u2;
} RI_renderable_triangle; 

typedef struct {
    RI_renderable_triangle* renderable_triangles;
    RI_triangle* triangles;
    RI_vertex* original_verticies;
    MU_vec4d* verticies;
    RI_normal* original_normals;
    RI_normal* normals;
    RI_uv* uvs;
    u32 triangle_count;
    u32 vertex_count;
    u32 normal_count;
    u32 uv_count;
} RI_mesh;

typedef PM_image RI_texture;

typedef struct {
    char* name;
    double ns; // shininess exponent
    MU_vec3d ka; // ambient
    MU_vec3d kd; // diffuse
    MU_vec3d ks; // specular
    MU_vec3d ke; // emission
    double ni; // optical density
    double d; // dissolve
    u8 illum; // illumination model
    RI_texture* map_Ka; // ambient map
    RI_texture* map_Kd; // diffuse map
                        // main texture/albedo map
    RI_texture* map_Ks; // specular map
    RI_texture* map_Ns; // optical density map
    RI_texture* map_d; // dissolve map
    RI_texture* disp; // displacement map
    RI_texture* decal; // deform map
    RI_texture* bump; // bump map
} RI_mtl; 

typedef struct {
    RI_texture* texture; 
    RI_texture* normal_map;
    u16 current_frame;
} RI_material;

typedef struct {
    MU_vec3d x;
    MU_vec3d y;
    MU_vec3d z;
} RI_basis_vectors;

typedef struct {
    RI_position position;
    RI_scale scale;
    RI_quaternion rotation;
    RI_basis_vectors basis;
} RI_transform;

typedef struct {
    MU_matrix translation_matrix;
    MU_matrix rotation_matrix;
    MU_matrix scaling_matrix;
    MU_matrix final_matrix;
} RI_actor_matricies;

typedef struct {
    RI_material* material; 
    RI_mesh* mesh;
    RI_transform transform;
    RI_actor_matricies matricies;
    void** children;
    void* parent;
    u8 hidden;
} RI_actor;

typedef struct {
    RI_transform transform; 
    double min_clip, max_clip;
    double FOV;
} RI_camera;

typedef struct {
    void** actors; // KT dyamic array
    void** cameras; // KT dynamic array
    u32 frames_rendered;
    MU_matrix camera_rotation_matrix;
    MU_matrix camera_translation_matrix;
    MU_matrix perspective_matrix;
} RI_scene;

typedef struct {
    u16 width, height;
    u16 half_width, half_height;
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Surface* sdl_surface; 
    int sdl_pitch;
    double* z_buffer;
} RI_window;

typedef struct {
    RI_mesh* default_mesh; 
    RI_material* default_material;
    RI_texture* default_texture;
    MU_matrix identity_matrix;
    MU_matrix intermidiate_matrix_a;
    MU_matrix intermidiate_matrix_b;
    MU_matrix intermidiate_matrix_c;
} RI_context;

#endif