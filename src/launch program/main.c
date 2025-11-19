#include "../headers/rasteriver.h"
#include <time.h>

int main(){
    RI_context* context = RI_get_context();
        
    context->window.width = 600;
    context->window.height = 600;
    context->window.title = "This is RasterIver 3.0!!!!!!!";
    
    if (RI_init() != 0){
        printf("failed to init RI\n");
        
        return 1;
    }

    RI_scene *scene = RI_new_scene();

    scene->camera.FOV = 1.5;
    scene->camera.min_clip = 0.1;

    RI_mesh *cube_mesh = RI_load_mesh("objects/cube.obj");

    int actor_count = 10 * 10;

    scene->actors = malloc(sizeof(RI_actor) * actor_count);

    float min_x = -100;
    float max_x = 100;
    float min_y = -100;
    float max_y = 100;

    for (int i = 0; i < (int)sqrt(actor_count); ++i){
        for (int j = 0; j < (int)sqrt(actor_count); ++j){
            scene->actors[i * (int)sqrt(actor_count) + j] = RI_new_actor();
         
            scene->actors[i * (int)sqrt(actor_count) + j]->mesh = cube_mesh;
         
            float offset_x = fabs(min_x - max_x) / ((int)sqrt(actor_count) - 1) * i;
            float offset_y = fabs(min_y - max_y) / ((int)sqrt(actor_count) - 1) * j;

            scene->actors[i * (int)sqrt(actor_count) + j]->scale = (RI_vector_3){100, 100, 100};
            scene->actors[i * (int)sqrt(actor_count) + j]->position = (RI_vector_3){
                min_x + offset_x, 
                min_y + offset_y, 
                600
            };
        }
    }

    scene->length_of_actors_array = actor_count;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;
    double delta_min = 0.00001;
    double delta_max = 100000;
    while (context->is_running){
        start = clock();
        
        // scene->camera.FOV = context->current_frame;
        
        for (int i = 0; i < (int)sqrt(actor_count); ++i){
            for (int j = 0; j < (int)sqrt(actor_count); ++j){
                RI_euler_rotation_to_quaternion(&scene->actors[i * (int)sqrt(actor_count) + j]->rotation, (RI_vector_3){context->current_frame * 0.01 * (i + 1), context->current_frame * 0.01 * (j + 1), context->current_frame * 0.001});
                

            }
        }

        RI_render(NULL, scene);

        RI_tick();

        end = clock();

        delta_time = fmin(fmax((double)(end - start) / (double)(CLOCKS_PER_SEC), delta_min), delta_max);
        fps = 1.0 / delta_time;

        total_fps += fps;

        printf("fps: %f average fps: %f\n", fps, total_fps / context->current_frame);
    }

    free(scene->actors);

    return 0;
}