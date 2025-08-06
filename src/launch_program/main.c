#include "../headers/rasteriver.h"

int main(){
    RI_init(800, 800, "This is RasterIver 2.0!!");

    int running = 1;

    char *filenames[] = {"objects/cube.obj"};

    RI_request_objects(1, filenames);

    while (running){
        RI_tick();
    }
}