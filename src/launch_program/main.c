#include "../headers/rasteriver.h"

int main(){
    RI_init(400, 400, "This is RasterIver 2.0!!");

    int running = 1;

    // get RasterIver context
    RasterIver *ri = RI_get_ri();

    // data for loading files
    char *filenames[] = {"objects/unit_cube.obj", "objects/test_guy.obj", "objects/unit_plane.obj"};
    RI_texture_creation_data texture_creation_info[2] = {{"textures/bill_mcdinner.png", {0, 0}}, {"textures/test_guy_texture.png", {0, 0}}};
    
    // requesting assets
    RI_mesh* meshes = RI_request_meshes(3, filenames, 0);
    RI_texture* textures = RI_request_textures(2, texture_creation_info);
    RI_material* materials = RI_request_materials(4);
    RI_actor* actors = RI_request_actors(4);
    RI_scene* scene = RI_request_scene();

    // meshes
    RI_mesh* unit_plane_mesh = &meshes[2];
    RI_mesh* unit_cube_mesh = &meshes[0];

    // textures
    RI_texture* bill_cube_texture = &textures[0];

    // materials
    RI_material* floor_material = &materials[0];
    floor_material->flags = 0;
    floor_material->albedo = 0xFF77FF77;
    
    RI_material* wall_material = &materials[1];
    wall_material->flags = 0;
    wall_material->albedo = 0xFF7777FF;

    RI_material* bill_cube_material = &materials[2];
    bill_cube_material->flags = RI_MATERIAL_HAS_TEXTURE;
    bill_cube_material->texture_reference = bill_cube_texture;

    RI_material* screen_material = &materials[3];
    screen_material->flags = RI_MATERIAL_HAS_TEXTURE;
    screen_material->texture_reference = ri->frame_buffer;

    // actors
    RI_actor* floor = &actors[0];
    floor->material_reference = floor_material;
    floor->mesh_reference = unit_plane_mesh;
    floor->transform.scale = (RI_vector_3f){100, 100, 100};
    floor->transform.position = (RI_vector_3f){0, -100, 200};
    floor->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_actor* wall = &actors[1];
    wall->material_reference = wall_material;
    wall->mesh_reference = unit_plane_mesh;
    wall->transform.scale = (RI_vector_3f){100, 100, 100};
    wall->transform.position = (RI_vector_3f){0, 0, 300};
    wall->transform.rotation = (RI_vector_4f){0.70710678, 0.70710678, 0, 0};

    RI_actor* bill_cube = &actors[2];
    bill_cube->material_reference = bill_cube_material;
    bill_cube->mesh_reference = unit_cube_mesh;
    bill_cube->transform.scale = (RI_vector_3f){50, 50, 50};
    bill_cube->transform.position = (RI_vector_3f){-50, 100, 100};
    bill_cube->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_actor* screen = &actors[3];
    screen->material_reference = screen_material;
    screen->mesh_reference = unit_plane_mesh;
    screen->transform.scale = (RI_vector_3f){50, 50, 50};
    screen->transform.position = (RI_vector_3f){0, 0, 250};
    screen->transform.rotation = (RI_vector_4f){0.70710678, 0.70710678};

    RI_add_actors_to_scene(4, actors, scene);

    ri->FOV = 1.5; // 90 degrees in radians

    while (running){
        bill_cube->transform.position = (RI_vector_3f){sin(ri->frame * 0.1) * 50 - 100, sin(ri->frame * 0.2 + 0.4) * 50, sin(ri->frame * 0.1) * 10 + 200};
        
        scene->camera_position = (RI_vector_3f){cos(ri->frame * 0.07) * 50 * sin(ri->frame * 0.2), sin(ri->frame * 0.07) * 50 * sin(ri->frame * 0.2), -150};

        RI_render(scene, ri->frame_buffer);
    }
}