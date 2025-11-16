#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include <CL/cl.h>
#include "math.h"

typedef enum {
    ri_true = 1,
    ri_false = 0,
} ri_bool;

typedef struct { // A loaded texture file
    uint32_t *image_buffer;
    RI_vector_2i resolution;
} RI_texture;

typedef struct {
    RI_vector_3 position_0;
    RI_vector_3 position_1;
    RI_vector_3 position_2;

    RI_vector_3 normal_0;
    RI_vector_3 normal_1;
    RI_vector_3 normal_2;

    RI_vector_2 uv_0;
    RI_vector_2 uv_1;
    RI_vector_2 uv_2;

    cl_uint should_render;
} RI_face;

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
} RI_temp_face;

typedef struct {
    int face_count;
    int face_index;
    ri_bool has_normals;
    ri_bool has_uvs;
} RI_mesh;

typedef struct {
    double r, g, b;
} RI_color;

typedef struct {
    uint32_t albedo;
} RI_material;

typedef struct {
    RI_vector_3 position;
    RI_vector_4 rotation;
    RI_vector_3 scale;
    RI_mesh *mesh;
    ri_bool active;
    int material_index;
} RI_actor;

typedef struct {
    RI_vector_3 position_0, position_1, position_2;
    RI_vector_3 normal_0, normal_1, normal_2;
    RI_vector_2 uv_0, uv_1, uv_2;
    int min_screen_x, max_screen_x, min_screen_y, max_screen_y;
    int should_render;
    int shrunk;
    int split;
} RI_renderable_face;

typedef struct {
    RI_vector_3 position;
    RI_vector_4 rotation;
    float FOV, min_clip, max_clip;
} RI_camera;

typedef struct {
    RI_actor **actors;
    RI_camera camera;
    int length_of_actors_array, face_count;
} RI_scene;

typedef struct {
    size_t size;
    void *pointer;
    int reallocated_free;
    int reallocated_alloc;
    int freed;
    int allocated;
    int line;
} RI_memory_allocation;

typedef struct {
    int width, height, half_width, half_height;
    char* title;
} RI_window;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *frame_buffer_texture;
    uint32_t *frame_buffer_intermediate;
    RI_texture *frame_buffer;
    int pitch;
} RI_SDL;

typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_kernel rasterization_kernel;
    cl_kernel transformation_kernel;
    cl_mem renderable_faces_mem_buffer;
    cl_mem faces_mem_buffer;
    cl_mem frame_buffer_mem_buffer;
    cl_mem vertecies_mem_buffer;
    cl_mem normals_mem_buffer;
    cl_mem uvs_mem_buffer;
    RI_renderable_face *faces_to_render;
    RI_face *faces;
    RI_temp_face *temp_faces;
    RI_vector_3 *temp_vertecies;
    RI_vector_3 *temp_normals;
    RI_vector_2 *temp_uvs;
    int face_count;
    int vertex_count;
    int normal_count;
    int uv_count;
    int length_of_renderable_faces_array;
} RI_CL;

typedef struct {
    ri_bool debug_memory;
    RI_memory_allocation *allocation_table;
    int current_allocation_index;
    int allocation_search_limit;
    int allocation_table_length;
} RI_memory;

typedef struct {
    RI_actor *default_actor;
} RI_defaults;

typedef struct {
    RI_window window;
    RI_SDL sdl;
    RI_CL opencl;
    RI_memory memory;
    RI_defaults defaults;
    ri_bool is_running;
    char* debug_prefix;
    ri_bool should_debug;   
    int current_renderable_face_index;
    int current_split_renderable_face_index;
    int current_frame;
} RI_context;

#endif