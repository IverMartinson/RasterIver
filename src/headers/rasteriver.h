#ifndef RASTERIVER_H
#define RASTERIVER_H

#include "functions.h"
#include "values.h"
#include "custom_types.h"
#include "math.h"
#include "sourparse.h"
#include <SDL2/SDL.h>
#include <CL/cl.h>

typedef struct {
    // rendering (non SDL)
    RI_texture *frame_buffer;
    double *z_buffer;
    RI_vector_2 z_buffer_resolution;
 
    double (*default_fragment_shader) (int pixel_x, int pixel_y, RI_vector_3f v_pos_0, RI_vector_3f v_pos_1, RI_vector_3f v_pos_2, RI_vector_3f normal, RI_vector_2f uv, uint32_t color);
    void (*default_vertex_shader) (RI_vector_3f *v_pos_0, RI_vector_3f *v_pos_1, RI_vector_3f *v_pos_2, double horizontal_fov_factor, double vertical_fov_factor);
    void (*perspective_vertex_shader) (RI_vector_3f *v_pos_0, RI_vector_3f *v_pos_1, RI_vector_3f *v_pos_2, double horizontal_fov_factor, double vertical_fov_factor);
    void (*orthographic_vertex_shader) (RI_vector_3f *v_pos_0, RI_vector_3f *v_pos_1, RI_vector_3f *v_pos_2, double horizontal_fov_factor, double vertical_fov_factor);

    int window_width;
    int window_height;
    char *window_title;

    // SDL specific
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    // RasterIver
    RI_mesh *loaded_meshes;
    RI_texture *loaded_textures;
    RI_material *materials;
    RI_actor *actors;
    RI_scene *scenes;

    RI_texture error_texture;
    RI_texture error_bump_map;
    RI_texture error_normal_map;
    RI_material error_material;
    RI_mesh error_mesh;

    // miscellaneous
    int loaded_mesh_count;
    int loaded_texture_count;
    int material_count;
    int actor_count;
    int scene_count;
    int running;
    int frame;
    char* prefix;

    // memory manager
    RI_memory_allocation* allocation_table;
    int debug_memory;
    int allocation_search_limit;
    int current_allocation_index;
    int allocation_table_length;

    // OpenCL
    cl_platform_id cl_platform;
    cl_uint cl_number_of_platforms;
    
    cl_device_id cl_device;
    cl_uint cl_number_of_devices;

    cl_context cl_context;
    cl_command_queue cl_command_queue;
} RasterIver;

RasterIver* RI_get_ri();

#endif