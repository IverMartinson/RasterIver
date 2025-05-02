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
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 0);
    RI_SetFlag(RI_FLAG_SHOW_FPS, 1);
    RI_SetFlag(RI_FLAG_SHOW_Z_BUFFER, 0);
    RI_SetFlag(RI_FLAG_CLEAN_POLYGONS, 1);
    RI_SetFlag(RI_FLAG_POPULATE_POLYGONS, 0);
    RI_SetFlag(RI_FLAG_BE_MASTER_RENDERER, 1);
    RI_SetFpsCap(120);

    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_newObject object_buffer[5] = {
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "gourd.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 200, 200, 200, "obj_file.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "obj_file.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "obj_file.obj", "texture.png"},
        {400, 400, 400, 0, 0, 0, 100, 100, 100, "obj_file.obj", "texture.png"},
    };

    RI_RequestObjects(object_buffer, 1);

    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}