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
    
    context->should_debug = ri_true;
    
    RI_scene *scene = RI_new_scene();

    scene->camera.FOV = 1.5;
    scene->camera.min_clip = 0.01;

    RI_actor *cube = RI_new_actor();
    RI_actor *triangle = RI_new_actor();

    RI_load_mesh("objects/homer.obj", cube);
    RI_load_mesh("objects/teapot.obj", triangle);

    cube->scale = (RI_vector_3){80, 80, 80};
    cube->position = (RI_vector_3){-20, -30, 100};
    cube->rotation = (RI_vector_4){1, 0, 0, 0};

    triangle->scale = (RI_vector_3){10, 10, 10};
    triangle->position = (RI_vector_3){20, 0, 100};
    triangle->rotation = (RI_vector_4){1, 0, 0, 0};
    
    scene->actors = malloc(sizeof(RI_actor) * 10);

    scene->actors[0] = cube;
    scene->actors[1] = triangle;

    scene->length_of_actors_array = 2;

    long int start, end;
    double fps = 0;

    double delta_time = 0;
    double delta_min = 0.00001;
    double delta_max = 1;
    while (context->is_running){
        start = clock();
        
        // scene->camera.FOV = context->current_frame;

        RI_render(NULL, scene);

        RI_euler_rotation_to_quaternion(&triangle->rotation, (RI_vector_3){context->current_frame * 0.01, context->current_frame * 0.1, context->current_frame * 0.01});
        RI_euler_rotation_to_quaternion(&cube->rotation, (RI_vector_3){context->current_frame * 0.0, context->current_frame * 0.1, context->current_frame * 0.0});
        
        RI_tick();

        end = clock();

        delta_time = fmin(fmax((double)(end - start) / (double)(CLOCKS_PER_SEC), delta_min), delta_max);
        fps = 1.0 / delta_time;

        printf("%f\n", fps);
    }

    free(scene->actors);

    return 0;
}