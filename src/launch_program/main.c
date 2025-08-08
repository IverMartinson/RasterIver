#include "../headers/rasteriver.h"

int main(){
    RI_init(300, 300, "This is RasterIver 2.0!!");

    int running = 1;

    char *filenames[] = {"objects/cube.obj", "objects/test_guy.obj"};
    RI_mesh* meshes = RI_request_meshes(2, filenames);

    RI_texture_creation_data texture_creation_info[2] = {{"textures/bill_mcdinner.png", {0, 0}}, {"textures/test_guy_texture.png", {0, 0}}};
    RI_texture* textures = RI_request_textures(2, texture_creation_info);

    RI_actor_creation_data actor_creation_info[2] = {{&meshes[1], NULL}, {&meshes[1], NULL}};
    RI_actor* actors = RI_request_actors(2, actor_creation_info);

    RI_scene* scene = RI_request_scene();

    RI_add_actors_to_scene(2, actors, scene);

    actors[0].transform.position = (RI_vector_3f){50, 0, 130};
    actors[0].transform.scale = (RI_vector_3f){50, 50, 50};
    actors[1].transform.scale = (RI_vector_3f){50, 50, 50};

    RasterIver *ri = RI_get_ri();
    ri->FOV = 60;

    while (running){
        actors[1].transform.position = (RI_vector_3f){0, 0, sin(ri->frame * 0.1) * 50 + 200};
        
        RI_render(scene);
    }
}