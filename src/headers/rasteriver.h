#ifndef RASTERIVER_H
#define RASTERIVER_H

#include "functions.h"
#include "values.h"
#include "custom_types.h"
#include "math.h"
#include <SDL2/SDL.h>

typedef struct {
    // rendering (non SDL)
    int window_width;
    int window_height;
    char *window_title;

    // SDL specific
    uint32_t *frame_buffer;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    // RasterIver
    RI_vertex *verticies; //data type that holds info about a vertex (positon, normal, UV coord)

    RI_vector_3f *loaded_object_vetex_positions; // original vertex positions from a loaded object
    RI_vector_3f *normals; // original normal vectors from a loaded object
    RI_vector_2f *uvs; // UV coords from a loaded object
    
    RI_vector_3f *transformed_vertex_positions; // vertex positions after trasformations (object & camera rotation & position changes, scale, etc)
    RI_vector_3f *transformed_normals; // normal vectors after rotation transformations (otherwise a normal's space would constantly be in local space instead of world space)

    RI_object_data *loaded_objects;

    // miscellaneous
    int loaded_object_count;
    int running;
    int frame;
    char* prefix;
} RasterIver;

#endif