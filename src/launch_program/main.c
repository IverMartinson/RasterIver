#include "../headers/rasteriver.h"

int main(){
    SP_font *comic_sans = SP_load_font("fonts/ComicSans-Regular.ttf");
    SP_font *cal_sans = SP_load_font("fonts/CalSans-Regular.ttf");
    SP_font *QwitcherGrypen = SP_load_font("fonts/QwitcherGrypen-Bold.ttf");
    
    // get RasterIver context
    RasterIver *ri = RI_get_ri();
    ri->prefix = "--------------------------";

    ri->debug_memory = 0;

    RI_init(700, 700, "This is RasterIver 2.0!!");

    int running = 1;

    // data for loading files
    char *filenames[] = {"objects/unit_plane.obj"};

    // requesting assets
    RI_mesh* meshes = RI_request_meshes(1, filenames, 0);
    RI_material* materials = RI_request_materials(1);
    RI_actor* actors = RI_request_actors(1);
    RI_scene* scenes = RI_request_scenes(1);

    RI_scene* scene = &scenes[0];

    // meshes
    RI_mesh* text_plane_mesh = &meshes[0];

    // materials
    RI_material* text_plane_material = &materials[0];
    text_plane_material->flags = RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;
    text_plane_material->texture_reference = RI_request_empty_texture((RI_vector_2){400, 400});
    text_plane_material->albedo = 0xFFFFFFFF;

    // actors
    RI_actor* text_plane = &actors[0];
    text_plane->material_reference = text_plane_material;
    text_plane->mesh_reference = text_plane_mesh;
    text_plane->transform.scale = (RI_vector_3f){50, 50, 50};
    text_plane->transform.position = (RI_vector_3f){100, 50, 400};
    text_plane->transform.rotation = (RI_vector_4f){0, 1, 0, 0};

    RI_add_actors_to_scene(1, actors, scene);

    scene->FOV = 1.5; // 90 degrees in radians
    scene->minimum_clip_distance = 0.1;

    RI_euler_rotation_to_quaternion(&scene->camera_rotation, (RI_vector_3f){0, 0, 0});

    double y_rotation = 0;

    scene->antialiasing_subsample_resolution = 8;
    scene->flags = RI_SCENE_DONT_USE_AA;
    
    RI_clear_texture(text_plane_material->texture_reference);

    RI_render_text(comic_sans, text_plane_material->texture_reference, (RI_vector_2f){0, 0}, 0xFFFFFFFF, 2, 80, "WOWWWW!!!11!!");
    RI_render_text(cal_sans, text_plane_material->texture_reference, (RI_vector_2f){0, 200}, 0xFFFFFFFF, 2, 80, "Wow!!");
    RI_render_text(QwitcherGrypen, text_plane_material->texture_reference, (RI_vector_2f){0, 300}, 0xFFFFFFFF, 2, 80, "WOW!!!!");    

    while (running){
        RI_euler_rotation_to_quaternion(&text_plane->transform.rotation, (RI_vector_3f){-1.5, ri->frame / 10.0, 0});

        RI_render(scene, ri->frame_buffer, 1);
        
        RI_render_text(comic_sans, ri->frame_buffer, (RI_vector_2f){00, 250}, 0xFFFFFFFF, 2, 80, "I love Comic Sans");
        RI_render_text(cal_sans, ri->frame_buffer, (RI_vector_2f){00, 350}, 0xFFFFFFFF, 2, 80, "I love Cal Sans");
        RI_render_text(QwitcherGrypen, ri->frame_buffer, (RI_vector_2f){00, 450}, 0xFFFFFFFF, 2, 80, "I love Qwitcher Grypen");

        RI_tick();

        ++ri->frame;
    }

    SP_free_font(comic_sans);
    SP_free_font(cal_sans);
    SP_free_font(QwitcherGrypen);
}