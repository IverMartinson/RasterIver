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

    scene->actors = malloc(sizeof(RI_actor));

    float min_x = -100;
    float max_x = 100;
    float min_y = -100;
    float max_y = 100;

    RI_texture* texture = RI_load_image("textures/test_texture_4_cube.bmp");

    scene->actors[0] = RI_new_actor();
         
    scene->actors[0]->mesh = cube_mesh;
    scene->actors[0]->texture = texture;
         
    scene->actors[0]->scale = (RI_vector_3){50, 50, 50};
    scene->actors[0]->position = (RI_vector_3){0, 0, 300};
    
    scene->length_of_actors_array = 1;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;
    double delta_min = 0.0001;
    double delta_max = 100000;
    
    double rotation = 0;
    
    while (context->is_running){
        start = clock();
        
        // scene->camera.FOV = context->current_frame;
        
        RI_euler_rotation_to_quaternion(&scene->actors[0]->rotation, (RI_vector_3){rotation, rotation, rotation});

        rotation += delta_time;

        RI_render(scene);

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