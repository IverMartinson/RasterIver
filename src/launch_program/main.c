#include "../headers/rasteriver.h"
#include <time.h>

double shader_function(int pixel_x, int pixel_y, RI_vector_3f v_pos_0, RI_vector_3f v_pos_1, RI_vector_3f v_pos_2, RI_vector_3f normal, RI_vector_2f uv, uint32_t color){
    if (uv.y > uv.x * uv.x) return 1;
    else return 0;
}

int main(){
    SP_font *comic_sans = SP_load_font("fonts/ComicSans-Regular.ttf");
    SP_font *cal_sans = SP_load_font("fonts/CalSans-Regular.ttf");
    SP_font *QwitcherGrypen = SP_load_font("fonts/QwitcherGrypen-Bold.ttf");
    
    // get RasterIver context
    RasterIver *ri = RI_get_ri();

    ri->debug_memory = 0;

    RI_init(700, 700, "This is RasterIver 2.0!!");

    // data for loading files
    char *filenames[] = {"objects/unit_plane.obj"};

    // requesting assets
    RI_mesh* meshes = RI_request_meshes(1, filenames, 0);
    RI_material* materials = RI_request_materials(2);
    RI_actor* actors = RI_request_actors(2);
    RI_scene* scenes = RI_request_scenes(1);

    RI_scene* scene = &scenes[0];

    // meshes
    RI_mesh* plane_mesh = &meshes[0];

    // materials
    RI_material* text_plane_material = &materials[0];
    text_plane_material->flags = RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;
    text_plane_material->texture_reference = RI_request_empty_texture((RI_vector_2){400, 400});
    text_plane_material->albedo = 0xFFFFFFFF;
    text_plane_material->fragment_shader = shader_function;

    RI_material* bill_material = &materials[1];
    bill_material->flags = RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;
    RI_texture_creation_data tex_data[1] = {(RI_texture_creation_data){"textures/THIS IS THE WALL.png", {0, 0}}};
    bill_material->texture_reference = RI_request_textures(1, tex_data);
    bill_material->albedo = 0xFFFFFFFF;

    // actors
    RI_actor* text_plane = &actors[0];
    text_plane->material_reference = text_plane_material;
    text_plane->mesh_reference = plane_mesh;
    text_plane->transform.scale = (RI_vector_3f){300, 300, 300};
    text_plane->transform.position = (RI_vector_3f){0, 0, 400};
    text_plane->transform.rotation = (RI_vector_4f){0, 1, 0, 0};
    RI_euler_rotation_to_quaternion(&text_plane->transform.rotation, (RI_vector_3f){-3.1415926 / 2, 0, 0});
    
    RI_actor* bill_plane = &actors[1];
    bill_plane->material_reference = bill_material;
    bill_plane->mesh_reference = plane_mesh;
    bill_plane->transform.scale = (RI_vector_3f){300, 300, 300};
    bill_plane->transform.position = (RI_vector_3f){0, 0, 600};
    bill_plane->transform.rotation = (RI_vector_4f){0, 1, 0, 0};
    RI_euler_rotation_to_quaternion(&bill_plane->transform.rotation, (RI_vector_3f){3.1415926 / 2, 0, 0});

    RI_vector_4f rotation_delta;

    RI_add_actors_to_scene(2, actors, scene);

    scene->FOV = 1.5; // 90 degrees in radians
    scene->minimum_clip_distance = 0.1;

    RI_euler_rotation_to_quaternion(&scene->camera_rotation, (RI_vector_3f){0, 0, 0});

    scene->camera_position = (RI_vector_3f){0, 0, 0};

    double y_rotation = 0;

    scene->antialiasing_subsample_resolution = 8;
    scene->flags = RI_SCENE_DONT_USE_AA;
    
    RI_clear_texture(text_plane_material->texture_reference);

    RI_render_text(comic_sans, text_plane_material->texture_reference, (RI_vector_2f){0, 0}, 0xFFFFFFFF, 2, 80, "WOWWWW!!!11!!");
    RI_render_text(cal_sans, text_plane_material->texture_reference, (RI_vector_2f){0, 200}, 0xFFFFFFFF, 2, 80, "Wow!!");
    RI_render_text(QwitcherGrypen, text_plane_material->texture_reference, (RI_vector_2f){0, 300}, 0xFFFFFFFF, 2, 80, "WOW!!!!");    

    long int start, end;
    char *fps_string = (char *)malloc(64 * sizeof(char));
    double fps = 0;

    double delta_time = 0;
    double delta_min = 0.00001;
    double delta_max = 0.1;

    fps_string[0] = '\0';

    while (ri->running){
        start = clock();

        RI_euler_rotation_to_quaternion(&rotation_delta, (RI_vector_3f){2.0 * delta_time, 2.0 * delta_time, 2.0 * delta_time});

        quaternion_multiply(&text_plane->transform.rotation, rotation_delta);

        RI_render(scene, ri->frame_buffer, 1);

        snprintf(fps_string, 64, "%.2f", fps);
        RI_render_text(cal_sans, ri->frame_buffer, (RI_vector_2f){0, 670}, 0xFFFFFFFF, 2, 20, fps_string);

        RI_render_text(comic_sans, ri->frame_buffer, (RI_vector_2f){0, 300}, 0xFFFF00FF, 2, 60, "Traditionally Choice");
        RI_render_text(comic_sans, ri->frame_buffer, (RI_vector_2f){0, 120}, 0xFFFF00FF, 2, 60, "itititititititititititititit");
        
        snprintf(fps_string, 64, "%.6f", delta_time);
        RI_render_text(cal_sans, ri->frame_buffer, (RI_vector_2f){0, 640}, 0xFFFFFFFF, 2, 20, fps_string);

        RI_tick(1);

        end = clock();

        delta_time = fmin(fmax(((double)(end - start) / (double)(CLOCKS_PER_SEC)), delta_min), delta_max);
        fps = 1.0 / delta_time;
    }

    RI_stop(0);

    SP_free_font(comic_sans);
    SP_free_font(cal_sans);
    SP_free_font(QwitcherGrypen);
}