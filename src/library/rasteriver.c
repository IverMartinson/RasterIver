#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"

RasterIver ri;

int RI_tick(){
    // do rendering stuff
    if (ri.running){
        ri.frame_buffer[(ri.frame * 10) % (ri.window_width * ri.window_height)] += 100;

        SDL_UpdateTexture(ri.texture, NULL, ri.frame_buffer, ri.window_width * sizeof(uint32_t));

        SDL_RenderClear(ri.renderer);
        SDL_RenderCopy(ri.renderer, ri.texture, NULL, NULL);
    
        SDL_RenderPresent(ri.renderer);
    }
    else{
        RI_stop(0);
    }

    // handle SDL events
    while (SDL_PollEvent(&ri.event))
    {
        switch (ri.event.type)
        {
        case SDL_QUIT:
            ri.running = 0;
        }
    }

    ++ri.frame;

    return 0;
}

int opencl_init(){
    return 0;
}

int sdl_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.window_width = RI_window_width;
    ri.window_height = RI_window_height;
    ri.window_title = RI_window_title;

    ri.frame_buffer = malloc(sizeof(uint32_t) * ri.window_width * ri.window_height);

    SDL_Init(SDL_INIT_VIDEO);

    ri.window = SDL_CreateWindow(RI_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ri.window_width, ri.window_height, SDL_WINDOW_OPENGL);

    ri.renderer = SDL_CreateRenderer(ri.window, -1, SDL_RENDERER_ACCELERATED);

    ri.texture = SDL_CreateTexture(ri.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ri.window_width, ri.window_height);

    return 0;
}

int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.running = 1;

    opencl_init();

    sdl_init(RI_window_width, RI_window_height, RI_window_title);

    return 0;
}

int RI_stop(int result){
    exit(result);

    return 0;
}