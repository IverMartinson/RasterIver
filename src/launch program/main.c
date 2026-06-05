#include "../headers/rasteriver.h"
#include <time.h>

int main(){
    RI_init();

    RI_window* window = RI_init_window("RASTERIVER 5.0 !!!!! (software edition)", 1200, 800);

    RI_scene* scene = RI_new_scene();

    scene->cameras[0] = RI_new_camera();

    RI_actor* actor_a = RI_load_multi_object_mesh("objects/terrorist.obj");
    RI_actor* actor_b = RI_load_multi_object_mesh("objects/counterterrorist.obj");

    // RI_add_actor_to_scence(scene, actor_a);
    RI_add_actor_to_scence(scene, actor_b);

    actor_a->transform.position.z = 400;
    actor_a->transform.position.x = -100;
    actor_b->transform.position.z = 400;
    actor_b->transform.position.x = 100;

    ((RI_camera*)scene->cameras[0])->FOV = MU_PI_4;
    ((RI_camera*)scene->cameras[0])->min_clip = 1;
    ((RI_camera*)scene->cameras[0])->max_clip = 10000;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;
    double delta_time_acc = 0;

    
    ((RI_camera*)scene->cameras[0])->transform.rotation = MU_quaternion_rotate((MU_vec3d){0, 1, 0}, MU_PI_2, actor_a->transform.rotation);

    while(1){
        start = clock();
        
        ((RI_camera*)scene->cameras[0])->transform.rotation = MU_quaternion_rotate((MU_vec3d){0, 1, 0}, delta_time, ((RI_camera*)scene->cameras[0])->transform.rotation);
        
        RI_render(scene, 0, window);

        RI_present(window);

        end = clock();

        ((RI_camera*)scene->cameras[0])->transform.position.z = sin(delta_time_acc) * 150;
    ((RI_camera*)scene->cameras[0])->transform.position.y = 100;
    ((RI_camera*)scene->cameras[0])->transform.position.x = cos(delta_time_acc) * 150;

        delta_time = (double)(end - start) / (double)(CLOCKS_PER_SEC);
        fps = 1.0 / delta_time;
        total_fps += fps;

        delta_time_acc += delta_time;

        printf("frame %d fps: %f average fps: %f\n", scene->frames_rendered, fps, total_fps / scene->frames_rendered);
    }

    return 0;
}