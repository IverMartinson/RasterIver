#ifndef RI_FUNCTIONS_H
#define RI_FUNCTIONS_H

#include "RI_types.h"

RI_material* RI_new_material();

RI_scene* RI_new_scene();

RI_actor* RI_new_actor();

void RI_add_actor_to_scence(RI_scene* scene, RI_actor* actor);

RI_texture* RI_load_image(char* file_path, u16 frame_height, u16 frame_count);

RI_actor* RI_load_multi_object_mesh(char* file_path);

RI_mesh* RI_load_mesh(char* file_path);

void RI_render(RI_scene* scene, u8 camera_index, RI_window* window);

void RI_present(RI_window* window);

RI_window* RI_init_window(char* title, u16 width, u16 height);

RI_camera* RI_new_camera();

void RI_init();

#endif