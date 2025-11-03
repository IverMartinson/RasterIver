#include "../headers/rasteriver.h"
#include <time.h>

int main(){
    RI_context* context = RI_get_context();
    
    context->window.width = 800;
    context->window.height = 800;
    context->window.title = "This is RasterIver 3.0!!!!!!!";
    
    if (RI_init() != 0){
        printf("failed to init RI\n");
        return 1;
    }

    RI_scene *scene = RI_new_scene();

    RI_actor *actor = RI_new_actor();

    RI_load_mesh("objects/cube.obj", actor);
    
    actor->scale = (RI_vector_3){100, 100, 100};
    actor->position = (RI_vector_3){0, 0, 0};
    actor->rotation = (RI_vector_4){1, 0, 0, 0};

    scene->actors = malloc(sizeof(RI_actor) * 10);

    scene->actors[0] = actor;

    scene->length_of_actors_array = 1;

    long int start, end;
    double fps = 0;

    double delta_time = 0;
    double delta_min = 0.00001;
    double delta_max = 1;
    while (context->is_running){
        start = clock();
        
        RI_render(NULL, scene);

        actor->position = (RI_vector_3){0, 0, 1000};

        RI_euler_rotation_to_quaternion(&actor->rotation, (RI_vector_3){context->current_frame * 0.001, context->current_frame * 0.001, context->current_frame * 0.001});
        
        RI_tick();

        end = clock();

        delta_time = fmin(fmax((double)(end - start) / (double)(CLOCKS_PER_SEC), delta_min), delta_max);
        fps = 1.0 / delta_time;

        printf("%f\n", fps);
    }

    free(scene->actors);

    return 0;
}