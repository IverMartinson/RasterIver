#ifndef RASTERIVER_H
#define RASTERIVER_H

#include "functions.h"
#include "values.h"
#include "custom_types.h"
#include "math.h"
#include <SDL2/SDL.h>

typedef struct {
    // rendering (non SDL)
    uint32_t *frame_buffer;
    float *z_buffer;
 
    int window_width;
    int window_height;
    char *window_title;

    float FOV;

    // SDL specific
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    // RasterIver
    RI_vertex *verticies; //data type that holds info about a vertex (positon, normal, UV coord)

    RI_vector_3f *loaded_mesh_vetex_positions; // original vertex positions from a loaded mesh
    RI_vector_3f *normals; // original normal vectors from a loaded mesh
    RI_vector_2f *uvs; // UV coords from a loaded mesh
    
    RI_vector_3f *transformed_vertex_positions; // vertex positions after trasformations (mesh & camera rotation & position changes, scale, etc)
    RI_vector_3f *transformed_normals; // normal vectors after rotation transformations (otherwise a normal's space would constantly be in local space instead of world space)

    RI_mesh *loaded_meshes;
    RI_texture *loaded_textures;
    RI_actor *actors;

    RI_texture default_texture;
    RI_material default_material;

    // miscellaneous
    int loaded_mesh_count;
    int loaded_texture_count;
    int actor_count;
    int running;
    int frame;
    char* prefix;
} RasterIver;

RasterIver* RI_get_ri();

#endif