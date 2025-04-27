#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"
#include <time.h>
#include <stdlib.h>

int main(){ 
    srand(time(NULL));                                                         

    RI_SetFlag(RI_FLAG_DEBUG, 1);
    RI_SetFlag(RI_FLAG_DEBUG_VERBOSE, 0);
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 1);
    RI_SetFlag(RI_FLAG_SHOW_Z_BUFFER, 0);
    RI_SetFlag(RI_FLAG_CLEAN_POLYGONS, 1);
    RI_SetFpsCap(60);
    
    if (RI_Init(800, 800, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_polygons polygons = RI_RequestPolygons(rand() % 10000);  

    polygons[0] = 622;
    polygons[1] = 213;
    polygons[2] = 123;
    
    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}