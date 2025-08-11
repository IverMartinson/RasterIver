#include "../headers/rasteriver.h"

int main(){
    // get RasterIver context
    RasterIver *ri = RI_get_ri();

    ri->debug_memory = 0;

    RI_init(1000, 1000, "This is RasterIver 2.0!!");

    int running = 1;

    // data for loading files
    char *filenames[] = {"objects/unit_cube.obj", "objects/cow-nonormals.obj", "objects/test_guy.obj", "objects/unit_plane.obj"};
    RI_texture_creation_data texture_creation_info[4] = {{"textures/bill_mcdinner.png", {0, 0}}, {"textures/this is the floor.png", {0, 0}}, {"textures/test_guy_texture.png", {0, 0}}, {"textures/THIS IS THE WALL.png", {0, 0}}};
    
    // requesting assets
    RI_mesh* meshes = RI_request_meshes(4, filenames, 0);
    RI_texture* textures = RI_request_textures(4, texture_creation_info);
    RI_material* materials = RI_request_materials(4);
    RI_actor* actors = RI_request_actors(4);
    RI_scene* scenes = RI_request_scenes(1);

    RI_scene* scene = &scenes[0];

    // meshes
    RI_mesh* unit_plane_mesh = &meshes[3];
    RI_mesh* test_object_mesh = &meshes[0];

    // textures
    RI_texture* test_object_texture = &textures[0];
    RI_texture* floor_texture = &textures[1];
    RI_texture* wall_texture = &textures[3];

    // materials
    RI_material* floor_material = &materials[0];
    floor_material->flags = RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED | RI_MATERIAL_USE_UV_LOOP_MULTIPLIER;
    floor_material->texture_reference = floor_texture;
    floor_material->albedo = 0xFFFFFFFF;
    floor_material->uv_loop_multiplier = (RI_vector_2f){25, 25};

    RI_material* wall_material = &materials[1];
    wall_material->flags = RI_MATERIAL_DOUBLE_SIDED | RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_USE_UV_RENDER_RESOLUTION;
    wall_material->albedo = 0xFF7777FF;
    wall_material->texture_reference = wall_texture;
    wall_material->texture_render_size = (RI_vector_2f){200, 400};

    RI_material* test_object_material = &materials[2];
    test_object_material->flags = RI_MATERIAL_HAS_TEXTURE;
    test_object_material->texture_reference = test_object_texture;
    test_object_material->albedo = 0xFFFFFFF;
    test_object_material->wireframe_width = 0.1;

    RI_material* screen_material = &materials[3];
    screen_material->flags = RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;
    screen_material->texture_reference = ri->frame_buffer;
    screen_material->albedo = 0xFFFFFFFF;

    // actors
    RI_actor* floor = &actors[0];
    floor->material_reference = floor_material;
    floor->mesh_reference = unit_plane_mesh;
    floor->transform.scale = (RI_vector_3f){1000, 100, 1000};
    floor->transform.position = (RI_vector_3f){0, -100, 200};
    floor->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_actor* wall = &actors[1];
    wall->material_reference = wall_material;
    wall->mesh_reference = unit_plane_mesh;
    wall->transform.scale = (RI_vector_3f){100, 100, 100};
    wall->transform.position = (RI_vector_3f){0, 0, 300};
    wall->transform.rotation = (RI_vector_4f){0.70710678, 0.70710678, 0, 0};

    RI_actor* test_object = &actors[2];
    test_object->material_reference = test_object_material;
    test_object->mesh_reference = test_object_mesh;
    test_object->transform.scale = (RI_vector_3f){50, 50, 50};
    test_object->transform.position = (RI_vector_3f){-50, 100, 100};
    test_object->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_actor* screen = &actors[3];
    screen->material_reference = screen_material;
    screen->mesh_reference = unit_plane_mesh;
    screen->transform.scale = (RI_vector_3f){50, 50, 50};
    screen->transform.position = (RI_vector_3f){0, 0, 250};
    screen->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_add_actors_to_scene(4, actors, scene);

    scene->FOV = 1.5; // 90 degrees in radians
    scene->minimum_clip_distance = 0.1;

    RI_euler_rotation_to_quaternion(&scene->camera_rotation, (RI_vector_3f){0, 0, 0});

    double y_rotation = 0;

    while (running){
        test_object->transform.position = (RI_vector_3f){sin(ri->frame * 0.1) * 50 - 100, sin(ri->frame * 0.2 + 0.4) * 50, sin(ri->frame * 0.1) * 10 + 200};

        RI_euler_rotation_to_quaternion(&screen->transform.rotation, (RI_vector_3f){-3.14159 / 2, 0, ri->frame * 0.03});

        // scene->camera_position = (RI_vector_3f){cos(ri->frame * 0.07) * 10 * sin(ri->frame * 0.2), sin(ri->frame * 0.07) * 10 * sin(ri->frame * 0.2), -300};
        scene->camera_position = (RI_vector_3f){0, 0, -300};
        
        wall->transform.scale.x = fabs(sin(y_rotation)) * 100 + 70;

        // RI_euler_rotation_to_quaternion(&scene->camera_rotation, (RI_vector_3f){0, y_rotation * .01 + 1.5, 0});

        // RI_euler_rotation_to_quaternion(&floor->transform.rotation, (RI_vector_3f){0, y_rotation, 0});
        
        RI_euler_rotation_to_quaternion(&test_object->transform.rotation, (RI_vector_3f){y_rotation, y_rotation, y_rotation});
        // RI_euler_rotation_to_quaternion(&test_object->transform.rotation, (RI_vector_3f){0, y_rotation, 0});

        y_rotation += 0.1;

        RI_render(scene, ri->frame_buffer);

        RI_tick();
    }
}