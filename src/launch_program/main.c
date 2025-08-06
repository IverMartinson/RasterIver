#include "../headers/rasteriver.h"

int main(){
    RI_init(800, 800, "This is RasterIver 2.0!!");

    int running = 1;

    while (running){
        RI_tick();
    }
}