#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"

// returns the RI_context
RI_context *RI_get_context();

// initilizes RasterIver
// returns completion code (0: fine, 1: error)
int RI_init();

// ticks RasterIver (updated window, check for events, renders scene, etc)
void RI_tick();

// renders a scene to the screen
void RI_render(RI_scene *scene);

// loads an OBJ file into memory as a mesh
RI_mesh *RI_load_mesh(char* filename);

// allocates and returns a pointer to a new scene
RI_scene *RI_new_scene();

// allocates and returns a pointer to a new actor
RI_actor *RI_new_actor();

// allocates and returns a pointer to a new material
RI_material *RI_new_material();

// allocates and returns a pointer to a new texture
RI_texture *RI_new_texture(int width, int height);

// loads an image file as a texture
RI_texture* RI_load_image(char* filename);

#endif