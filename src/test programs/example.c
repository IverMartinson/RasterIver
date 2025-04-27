#include <stdio.h>
#include "../RasterIver/headers/rasteriver.h"

int main(){
    RI_SetFlag(RI_FLAG_DEBUG, 1);
    RI_SetFlag(RI_FLAG_DEBUG_VERBOSE, 0);
    RI_SetFlag(RI_FLAG_DEBUG_FPS, 1);
    RI_SetFpsCap(60);
    
    if (RI_Init(800, 800, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_RequestPolygons(1000);

    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}