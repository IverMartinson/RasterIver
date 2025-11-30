#include "../headers/rasteriver.h"
#include <time.h>

int main(){
    RI_context* context = RI_get_context();
        
    context->window.width = 16 * 100;
    context->window.height = 9 * 100;
    context->window.title = "This is RasterIver 3.0!!!!!!!";
    
    if (RI_init() != 0){
        printf("failed to init RI\n");
        
        return 1;
    }
    
    RI_scene *scene = RI_new_scene();


    scene->camera.FOV = 1.5;
    scene->camera.min_clip = 0.1;

    context->opencl.tile_width = 64;
    context->opencl.tile_height = 36;

    RI_mesh *skybox_mesh = RI_load_mesh("objects/skybox.obj");
    RI_mesh *gordon_mesh = RI_load_mesh("objects/terrain.obj");
    RI_mesh *gordon_head_mesh = RI_load_mesh("objects/gordon_freeman_head.obj");
    RI_mesh *text_mesh = RI_load_mesh("objects/gordon_freeman.obj");
    RI_mesh *plane_mesh = RI_load_mesh("objects/plane.obj");

    scene->actors = malloc(sizeof(RI_actor) * 5);

    RI_texture* skybox_texture = RI_load_animation("textures/emoji_angry.bmp", 40);
    RI_texture* gordon_texture = RI_load_image("textures/terrain_texture.bmp");
    RI_texture* gordon_face_texture = RI_load_animation("textures/gordon_face_animated.bmp", 3);
    RI_texture* emoji_texture = RI_load_image("textures/gordon_body.bmp");
    RI_texture* iver_texture = RI_load_animation("textures/emoji_gif.bmp", 11);
    RI_texture* sky_texture = RI_load_image("textures/skybox_texture_4_cube_1024x1024.bmp");

    scene->actors[0] = RI_new_actor();
    scene->actors[1] = RI_new_actor();
    scene->actors[2] = RI_new_actor();
    scene->actors[3] = RI_new_actor();
    scene->actors[4] = RI_new_actor();

    context->window.aspect_mode = RI_ASPECT_MODE_LETTERBOX;

    scene->actors[1]->mesh = skybox_mesh;
    scene->actors[1]->texture = sky_texture;
    scene->actors[1]->scale = (RI_vector_3){1000, 1000, 1000};
    scene->actors[1]->position = (RI_vector_3){0, 0, 0};
// scene->actors[1]->active = 0;

    scene->actors[2]->mesh = plane_mesh;//gordon_mesh;
    scene->actors[2]->texture = gordon_texture;
    scene->actors[2]->scale = (RI_vector_3){2000, 1000, 2000};
    scene->actors[2]->position = (RI_vector_3){0, -30, 0};
    // scene->actors[2]->active = 0;

    scene->actors[3]->mesh = text_mesh;
    scene->actors[3]->texture = emoji_texture;
    scene->actors[3]->scale = (RI_vector_3){3, 1, 3};
    scene->actors[3]->position = (RI_vector_3){0, 900, -400};
// scene->actors[3]->active = 0;

    float scale = 30;

    scene->actors[0]->mesh = plane_mesh;
    scene->actors[0]->texture = skybox_texture;
    scene->actors[0]->scale = (RI_vector_3){scale * 1.261904762, scale, scale};
    scene->actors[0]->position = (RI_vector_3){60, 0, 180};

    scene->actors[4]->mesh = plane_mesh;
    scene->actors[4]->texture = iver_texture;
    scene->actors[4]->scale = (RI_vector_3){scale, scale, scale};
    scene->actors[4]->position = (RI_vector_3){60, 40, 180};
// scene->actors[4]->active = 0;

    RI_euler_rotation_to_quaternion(&scene->actors[0]->rotation, (RI_vector_3){3.14159 / 2, 0.3, 0});
    // RI_euler_rotation_to_quaternion(&scene->actors[1]->rotation, (RI_vector_3){0, 3.14159, 0});
    RI_euler_rotation_to_quaternion(&scene->actors[2]->rotation, (RI_vector_3){0, 0.78539816339, 0});

    scene->length_of_actors_array = 5;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;
    double delta_min = 0.0001;
    double delta_max = 100000;
    
    double rotation = 0;
    
    float animation_frame = 0;

    while (context->is_running){
        start = clock();
        
        // scene->camera.FOV = context->current_frame;
        
        scene->actors[4]->texture_frame = rotation * 20;
        scene->actors[0]->texture_frame = rotation * 15;


        RI_euler_rotation_to_quaternion(&scene->actors[4]->rotation, (RI_vector_3){3.14159 / 2, -fabs(fmod(rotation, 3.14159)) + -3.14159 / 2, 0});

        RI_euler_rotation_to_quaternion(&scene->actors[3]->rotation, (RI_vector_3){0, rotation / 4, 0.7});
// scene->actors[2]->position.z += delta_time;

        scene->actors[4]->position = (RI_vector_3){cos(fmod(rotation, 3.14159)) * 200, 0, sin(fmod(rotation, 3.14159)) * 200};

    scene->actors[0]->position = (RI_vector_3){60, fabs(sin(fabs(rotation) * 6)) * 10, 180};



        rotation += delta_time * .7;

        RI_render(scene);

        RI_tick();

        end = clock();

        delta_time = fmin(fmax((double)(end - start) / (double)(CLOCKS_PER_SEC), delta_min), delta_max);
        fps = 1.0 / delta_time;

        total_fps += fps;

        printf("frame %d fps: %f average fps: %f\n", context->current_frame, fps, total_fps / context->current_frame);
    }

    free(scene->actors);

    return 0;
}