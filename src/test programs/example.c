#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"
#include <time.h>
#include <stdlib.h>

int width = 800;
int height = 800;

int main(){ 
    srand(time(NULL));                                                         

    RI_SetFlag(RI_FLAG_DEBUG, 1);
    RI_SetFlag(RI_FLAG_DEBUG_VERBOSE, 0);
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 0);
    RI_SetFlag(RI_FLAG_SHOW_FPS, 1);
    RI_SetFlag(RI_FLAG_SHOW_Z_BUFFER, 0);
    RI_SetFlag(RI_FLAG_CLEAN_POLYGONS, 1);
    RI_SetFpsCap(120);
    
    if (RI_Init(width, height, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_polygons polygons = RI_RequestPolygons(1);  

    for (int p = 0; p < 9; p += 3){ 
        polygons[p] = rand() % width;
        polygons[p + 1] = rand() % height;
        polygons[p + 2] = rand() % ((width + height) / 2);
    }
    
    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}