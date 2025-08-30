#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "custom_types.h"
#include "sourparse.h"

// ----- request functions

// Load an object file
RI_mesh* RI_request_mesh(char *filename); 

// Allocate a new actor
RI_actor* RI_request_actor(); 

// Allocate a new material
RI_material* RI_request_material(); 

// Load a texture
RI_texture* RI_request_texture(char *filename); 

// Allocate a new texture without using an image file
RI_texture* RI_request_empty_texture(RI_vector_2 resolution); 

// Allocate a new scene
RI_scene* RI_request_scene(); 


// ----- master services

// Initialize RasterIver
int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title); 

// Stop RasterIver safely and free memory
int RI_stop(int result); 

// Tick RasterIver.
// Copies the window's texture (ri.frame_buffer.image_buffer) onto the screen, calls SDL_RenderPresent, handles SDL events, increases ri.frame by 1, and optionally clears the window's texture
void RI_tick(int clear_window_texture_after_rendering); 


// ----- rendering services

// Render a scene to a texture
int RI_render(RI_scene *scene, RI_texture *target_texture, int clear_texture); 

// Renders text to a texture
void RI_render_text(SP_font *font, RI_texture *target_texture, RI_vector_2f position, uint32_t color, int bezier_resolution, double size, char *text);

// Draws a line on a texture
void RI_draw_line(RI_texture *target_texture, RI_vector_2 point_a, RI_vector_2 point_b, uint32_t color);

// Sets all pixels in a texture to a color
void RI_wipe_texture(RI_texture *target_texture, uint32_t color);


// ----- memory

// Unallocated a scene
void RI_free_scene(RI_scene *scene);

// Unallocates a texture 
void RI_free_texture(RI_texture *texture);

// Unallocates a material (NOT it's references)
void RI_free_material(RI_material *material);

// Unallocated a mesh
void RI_free_mesh(RI_mesh *mesh);

// Unallocated an actor (NOT it's references)
void RI_free_actor(RI_actor *actor);

// ------ other

int RI_add_actors_to_scene(int RI_number_of_actors_to_add_to_scene, RI_actor **actors, RI_scene *scene);
void RI_euler_rotation_to_quaternion(RI_vector_4f* quaternion, RI_vector_3f euler_rotation);
RI_vector_2f v2_to_2f(RI_vector_2 v);
RI_vector_2 v2f_to_2(RI_vector_2f v);

#endif