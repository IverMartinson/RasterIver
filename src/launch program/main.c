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
    RI_SetFlag(RI_FLAG_HANDLE_SDL_EVENTS, 1);
    RI_SetFlag(RI_FLAG_SHOW_INFO, 0);
    RI_SetFlag(RI_FLAG_USE_CPU, 0);

    RI_SetValue(RI_VALUE_WIREFRAME_SCALE, 0.01);

    char prefix[50] = "[RASTERIVER IS AMAZING] ";
    RI_SetDebugPrefix(prefix);
    //RI_SetFpsCap(120);

    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_newObject object_buffer[9] = {
        {0, 0, 30,          0, 0, 0, 0,          10, 20, 5,       RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {50, 0, 100,        0, 0, 0, 0,          10, 10, 100,        RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {-50, 0, 100,         0, 0, 0, 0,          10, 10, 100,      RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {0, 50, 100,       0, 0, 0, 0,          10, 10, 100,          RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {0, -50, 100,       0, 0, 0, 0,          10, 10, 100,         RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {-50, 50, 100,       0, 0, 0, 0,          10, 10, 100,         RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {-50, -50, 100,          0, 0, 0, 0,          10, 10, 100,      RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {50, 50, 100,          0, 0, 0, 0,          10, 10, 100,       RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        {50, -50, 100,        0, 0, 0, 0,          10, 10, 100,        RI_PMP_TEXTURED, "objects/cube.obj", "textures/bill_mcdinner.png"},
        };

    int objects_to_request = 9;

    RI_objects objects = RI_RequestObjects(object_buffer, objects_to_request);

        for (int i = 0; i < objects_to_request; i++){
            objects[i].material.albedo.a = 255 * ((float)i / objects_to_request);
            objects[i].material.albedo.r = 255 * ((float)i / objects_to_request);
            objects[i].material.albedo.g = 255 * ((float)i / objects_to_request);
            objects[i].material.albedo.b = 255 * ((float)i / objects_to_request);
        }
        
    while (RI_IsRunning() == RI_RUNNING){        
        objects[0].transform.rotation.x += 0.1;
        objects[0].transform.rotation.y += 0.1;
        objects[0].transform.rotation.z += 0.1;

        RI_Tick();
    }

    RI_Stop();
}