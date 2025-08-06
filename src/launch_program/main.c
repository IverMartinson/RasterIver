#include "../headers/rasteriver.h"

int main(){
    RI_init(800, 800, "This is RasterIver 2.0!!");

    int running = 1;

    char *filenames[] = {"objects/cube.obj", "objects/test_guy.obj"};
    RI_mesh* meshes = RI_request_meshes(2, filenames);

    RI_texture_creation_data texture_creation_info[2] = {{"textures/bill_mcdinner.png", {0, 0}}, {"textures/test_guy_texture.png", {0, 0}}};
    RI_texture* textures = RI_request_textures(2, texture_creation_info);

    RI_actor_creation_data actor_creation_info[2] = {{&meshes[0], &textures[0]}, {&meshes[1], &textures[1]}};
    RI_actor* actors = RI_request_actors(2, actor_creation_info);

    while (running){
        RI_tick();
    }
}