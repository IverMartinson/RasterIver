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

    RI_mesh *skybox_mesh = RI_load_mesh("objects/skybox.obj");
    RI_mesh *gordon_mesh = RI_load_mesh("objects/gordon_freeman.obj");
    RI_mesh *gordon_head_mesh = RI_load_mesh("objects/gordon_freeman_head.obj");
    RI_mesh *plane_mesh = RI_load_mesh("objects/plane.obj");

    scene->actors = malloc(sizeof(RI_actor) * 3);

    RI_texture* skybox_texture = RI_load_image("textures/alley_skybox_3072x3072.bmp");
    RI_texture* gordon_texture = RI_load_image("textures/gordon_body.bmp");
    RI_texture* gordon_face_texture = RI_load_image("textures/gordon_face.bmp");

    scene->actors[0] = RI_new_actor();
    scene->actors[1] = RI_new_actor();
    scene->actors[2] = RI_new_actor();
         
    context->window.aspect_mode = RI_ASPECT_MODE_LETTERBOX;

    scene->actors[0]->mesh = skybox_mesh;
    scene->actors[0]->texture = skybox_texture;
    scene->actors[0]->scale = (RI_vector_3){10000, 10000, 10000};
    scene->actors[0]->position = (RI_vector_3){0, 0, 0};

    scene->actors[1]->mesh = gordon_head_mesh;
    scene->actors[1]->texture = gordon_face_texture;
    scene->actors[1]->scale = (RI_vector_3){1, 1, 1};
    scene->actors[1]->position = (RI_vector_3){0, -60, 150};

    scene->actors[2]->mesh = gordon_mesh;
    scene->actors[2]->texture = gordon_texture;
    scene->actors[2]->scale = (RI_vector_3){1, 1, 1};
    scene->actors[2]->position = (RI_vector_3){0, -60, 150};
    
    RI_euler_rotation_to_quaternion(&scene->actors[0]->rotation, (RI_vector_3){0, 0, 0});

    scene->length_of_actors_array = 3;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;
    double delta_min = 0.0001;
    double delta_max = 100000;
    
    double rotation = -1.5;
    
    while (context->is_running){
        start = clock();
        
        // scene->camera.FOV = context->current_frame;
        
        RI_euler_rotation_to_quaternion(&scene->actors[1]->rotation, (RI_vector_3){0, rotation, 0});
        RI_euler_rotation_to_quaternion(&scene->actors[2]->rotation, (RI_vector_3){0, rotation, 0});

        rotation += delta_time;

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