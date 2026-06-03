#include "../headers/rasteriver.h"
#include <time.h>

int main(){
    RI_init();

    RI_window* window = RI_init_window("RASTERIVER 5.0 !!!!! (software edition)", 800, 800);

    RI_scene* scene = RI_new_scene();

    scene->cameras[0] = RI_new_camera();

    RI_actor* actor_a = RI_new_actor();
    RI_actor* actor_b = RI_new_actor();
    RI_actor* actor_c = RI_new_actor();
    RI_actor* actor_d = RI_new_actor();
    RI_actor* actor_e = RI_new_actor();

    RI_material* material = RI_new_material();

    actor_a->mesh = RI_load_mesh("objects/gordon.obj");
    actor_b->mesh = RI_load_mesh("objects/CREEPY_JOHNSONS.obj");
    actor_c->mesh = RI_load_mesh("objects/gordon.obj");
    actor_d->mesh = RI_load_mesh("objects/gordon.obj");
    actor_e->mesh = RI_load_mesh("objects/gordon.obj");
    
    actor_b->material->texture = RI_load_image("textures/earth.bmp", 0, 0);

    RI_add_actor_to_scence(scene, actor_a);
    RI_add_actor_to_scence(scene, actor_b);
    RI_add_actor_to_scence(scene, actor_c);
    RI_add_actor_to_scence(scene, actor_d);
    RI_add_actor_to_scence(scene, actor_e);
    
// actor_a->transform.scale = (MU_vec3d){1, 1, 1};
// actor_b->transform.scale = (MU_vec3d){1, 1, 1};
// actor_c->transform.scale = (MU_vec3d){1, 1, 1};
// actor_d->transform.scale = (MU_vec3d){1, 1, 1};
// actor_e->transform.scale = (MU_vec3d){1, 1, 1};

    actor_a->transform.position.z = 400;
    actor_b->transform.position.z = 400;
    actor_c->transform.position.z = 400;
    actor_e->transform.position.z = 400;
    actor_d->transform.position.z = 400;
    actor_c->transform.position.x = 200;
    actor_a->transform.position.x = -200;
    actor_d->transform.position.x = 100;
    actor_e->transform.position.x = -100;

    ((RI_camera*)scene->cameras[0])->FOV = MU_PI_2;
    ((RI_camera*)scene->cameras[0])->min_clip = 1;
    ((RI_camera*)scene->cameras[0])->max_clip = 10000;

    long int start, end;
    double fps = 0;

    float total_fps = 0;

    double delta_time = 0;

    while(1){
        start = clock();
        
        actor_a->transform.rotation = MU_quaternion_rotate((MU_vec3d){0, 1, 0}, delta_time, actor_a->transform.rotation);
        ((RI_camera*)scene->cameras[0])->transform.rotation = MU_quaternion_rotate((MU_vec3d){0, 0, 1}, delta_time, ((RI_camera*)scene->cameras[0])->transform.rotation);
        actor_b->transform.rotation = actor_a->transform.rotation;
        actor_c->transform.rotation = actor_a->transform.rotation;
        actor_d->transform.rotation = actor_a->transform.rotation;
        actor_e->transform.rotation = actor_a->transform.rotation;
        
        RI_render(scene, 0, window);

        RI_present(window);

        end = clock();

        delta_time = (double)(end - start) / (double)(CLOCKS_PER_SEC);
        fps = 1.0 / delta_time;
        total_fps += fps;

        printf("frame %d fps: %f average fps: %f\n", scene->frames_rendered, fps, total_fps / scene->frames_rendered);
    }

    return 0;
}