#include <stdio.h>
#include "rasteriver.h"

int main(){
    RI_SetDebugFlag(1);
    
    if (RI_Init(800, 800, "Rasteriver Test") == RI_ERROR){
        return 1;
    }

    RI_RequestPolygons(100);

    while (RI_IsRunning() == RI_RUNNING){
        RI_Tick();
    }

    RI_Stop();
}