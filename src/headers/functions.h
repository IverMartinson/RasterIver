#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "custom_types.h"

RI_mesh* RI_request_meshes(int RI_number_of_requested_objects, char **filenames, int RI_request_just_mesh); // Load object file(s)
RI_actor* RI_request_actors(int RI_number_of_requested_actors); // Load texture file(s)
RI_material* RI_request_materials(int RI_number_of_requested_materials); // Create materials
RI_texture* RI_request_textures(int RI_number_of_requested_textures, RI_texture_creation_data *texture_creation_data); // Create new actor(s)
RI_scene* RI_request_scenes(int RI_number_of_requested_scenes);
int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title); // Initialize RasterIver
int RI_stop(int result); // Stop RasterIver safely and free memory
int RI_render(RI_scene *scene, RI_texture *target_texture, int clear_texture); // Render a scene to a texture
int RI_add_actors_to_scene(int RI_number_of_actors_to_add_to_scene, RI_actor *actors, RI_scene *scene);
void RI_euler_rotation_to_quaternion(RI_vector_4f* quaternion, RI_vector_3f euler_rotation);
void RI_tick();

#endif