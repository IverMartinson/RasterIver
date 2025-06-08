#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"
#include <time.h>
#include <stdlib.h>

int width = 200;
int height = 200;

int main(){ 
    srand(time(NULL));                                                         

    RI_SetFlag(RI_FLAG_DEBUG, 1);
    RI_SetFlag(RI_FLAG_DEBUG_LEVEL, RI_DEBUG_HIGH);
    RI_SetFlag(RI_FLAG_DEBUG_TICK, 0);
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 0);
    RI_SetFlag(RI_FLAG_SHOW_FPS, 1);
    RI_SetFlag(RI_FLAG_SHOW_FRAME, 0);
    RI_SetFlag(RI_FLAG_SHOW_BUFFER, RI_BUFFER_COMPLETE);
    RI_SetFlag(RI_FLAG_CLEAN_POLYGONS, 1);
    RI_SetFlag(RI_FLAG_POPULATE_POLYGONS, 0);
    RI_SetFlag(RI_FLAG_BE_MASTER_RENDERER, 1);
    RI_SetFlag(RI_FLAG_HANDLE_SDL_EVENTS, 0);
    RI_SetFlag(RI_FLAG_SHOW_INFO, 0);
    RI_SetFpsCap(120);

    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_newObject object_buffer[3] = {
        {0, 0, 300,       0, 0, 0, -9999999,          100, 100, 100,  "objects/rsotated_cube.obj", "textures/thistexturedoesntexist.png"},
        {0, 0, 15,         0, 0, 0, -9999999,  1, 1, 1,     "objects/test_guy_hd.obj", "textures/test_guy_texture.png"},
        {0, 0, 300,      0, 0.0, 0, -9999999,          50, 50, 50,  "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
    };

    RI_objects objects = RI_RequestObjects(object_buffer, 1);

    SDL_Event event;

    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                RI_Stop(0);
            }
        }
    }

    RI_Stop();
}