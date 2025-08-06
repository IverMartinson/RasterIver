#ifndef RASTERIVER_H
#define RASTERIVER_H

#include "functions.h"
#include "values.h"
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

    // miscellaneous
    int running;
    int frame;
} RasterIver;

#endif