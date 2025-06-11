#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"
#include <time.h>
#include <stdlib.h>

int width = 800;
int height = 800;

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

    char prefix[50] = "[RASTERIVER IS AMAZING] ";
    RI_SetDebugPrefix(prefix);
    //RI_SetFpsCap(120);

    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_newObject object_buffer[49] = {
        {0, 0, 100,          0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, 0, 100,        0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, 0, 100,         0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, 0, 100,       0, 0, 0, -9999999,          10, 10, 10,          "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, 0, 100,       0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, 0, 100,       0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, 0, 100,          0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, 10, 100,          0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, 10, 100,        0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, 10, 100,         0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, 10, 100,       0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, 10, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, 10, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, 10, 100,          0, 0, 0, -9999999,          10, 10, 10,     "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, -10, 100,          0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, -10, 100,        0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, -10, 100,         0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, -10, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, -10, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, -10, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, -10, 100,          0, 0, 0, -9999999,          10, 10, 10,    "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, 20, 100,          0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, 20, 100,        0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, 20, 100,         0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, 20, 100,       0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, 20, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, 20, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, 20, 100,          0, 0, 0, -9999999,          10, 10, 10,     "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, -20, 100,          0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, -20, 100,        0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, -20, 100,         0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, -20, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, -20, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, -20, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, -20, 100,          0, 0, 0, -9999999,          10, 10, 10,    "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, 30, 100,          0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, 30, 100,        0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, 30, 100,         0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, 30, 100,       0, 0, 0, -9999999,          10, 10, 10,         "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, 30, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, 30, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, 30, 100,          0, 0, 0, -9999999,          10, 10, 10,     "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {0, -30, 100,          0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {10, -30, 100,        0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {20, -30, 100,         0, 0, 0, -9999999,          10, 10, 10,      "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {30, -30, 100,       0, 0, 0, -9999999,          10, 10, 10,        "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-10, -30, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-20, -30, 100,       0, 0, 0, -9999999,          10, 10, 10,       "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
        {-30, -30, 100,          0, 0, 0, -9999999,          10, 10, 10,    "objects/rotated_cube.obj", "textures/bill_mcdinner.png"},
    };

    int objects_to_request = 49;

    RI_objects objects = RI_RequestObjects(object_buffer, objects_to_request);

    while (RI_IsRunning() == RI_RUNNING){
        for (int i = 0; i < objects_to_request; i++){
            objects[i].transform.rotation.x += 0.01;
            objects[i].transform.rotation.y += 0.02;
            objects[i].transform.rotation.z += 0.03;

            objects[i].transform.rotation.x = fmod(objects[i].transform.rotation.x, RI_2PI);
            objects[i].transform.rotation.y = fmod(objects[i].transform.rotation.y, RI_2PI);
            objects[i].transform.rotation.z = fmod(objects[i].transform.rotation.z, RI_2PI);
        }
        
        RI_Tick();
    }

    RI_Stop();
}