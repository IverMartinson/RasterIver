#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"
#include <time.h>
#include <stdlib.h>

int width = 800;
int height = 800;

int main(){ 
    srand(time(NULL));                                                         

    RI_SetFlag(RI_FLAG_DEBUG, 1);
    RI_SetFlag(RI_FLAG_DEBUG_VERBOSE, 1);
    RI_SetFlag(RI_FLAG_DEBUG_TICK, 0);
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 0);
    RI_SetFlag(RI_FLAG_SHOW_FPS, 1);
    RI_SetFlag(RI_FLAG_SHOW_FRAME, 1);
    RI_SetFlag(RI_FLAG_SHOW_BUFFER, RI_BUFFER_UV);
    RI_SetFlag(RI_FLAG_CLEAN_POLYGONS, 1);
    RI_SetFlag(RI_FLAG_POPULATE_POLYGONS, 0);
    RI_SetFlag(RI_FLAG_BE_MASTER_RENDERER, 1);
    RI_SetFlag(RI_FLAG_SHOW_INFO, 1);
    RI_SetFpsCap(120);

    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_newObject object_buffer[5] = {
        {600, 400, 300, 0, 0, 0, 100, 100, 100, "objects/rotated_cube.obj", "texture.png"},
        {400, 20, 400, 0, 0, 0, 400, 400, 400, "objects/gordon_freeman.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "objects/teapot.obj", "texture.png"},
        {200, 400, 400, 0, 0, 0, 100, 100, 100, "objects/gourd.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "objects/obj_file.obj", "texture.png"},
    };

    RI_objects objects = RI_RequestObjects(object_buffer, 1);

    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}