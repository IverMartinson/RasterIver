#include <stdio.h>
#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#include "stdint.h"
#include <math.h>

RasterIver ri;

void debug(char *string, ...){
    va_list args;
    va_start(args, string);

    char message[500];

    strcpy(message, ri.prefix);

    strcat(message, string);

    vprintf(message, args);
    printf("\n");

    va_end(args);
}

RasterIver* RI_get_ri(){
    return &ri;
}

int RI_add_actors_to_scene(int RI_number_of_actors_to_add_to_scene, RI_actor *actors, RI_scene *scene){
    int previous_actor_count = scene->actor_count;

    scene->actor_count += RI_number_of_actors_to_add_to_scene;

    scene->actors = realloc(scene->actors, sizeof(RI_actor *) * scene->actor_count);

    for (int i = 0; i < RI_number_of_actors_to_add_to_scene; ++i){
        scene->actors[i + previous_actor_count] = &actors[i];
    }

    return 0;
}

RI_scene* RI_request_scene(){
    RI_scene* new_scene = malloc(sizeof(RI_scene));

    new_scene->actor_count = 0;
    new_scene->actors = NULL;
    
    return new_scene;
}

RI_actor* RI_request_actors(int RI_number_of_requested_actors, RI_actor_creation_data *actor_creation_data){
    int previous_actor_count = ri.actor_count;
    ri.actor_count += RI_number_of_requested_actors;
    
    ri.actors = realloc(ri.actors, sizeof(RI_actor) * ri.actor_count);

    for (int i = 0; i < RI_number_of_requested_actors; ++i){
        RI_actor new_actor = {0};

        new_actor.mesh_reference = actor_creation_data[i].mesh_reference;
        
        if (actor_creation_data->material_reference){
            new_actor.material_reference = actor_creation_data[i].material_reference;
        }
        else {
            new_actor.material_reference = &ri.default_material;
        }

        ri.actors[i + previous_actor_count] = new_actor;
    }

    return ri.actors;
}

RI_texture* RI_request_textures(int RI_number_of_requested_textures, RI_texture_creation_data *texture_creation_data){
    int previous_loaded_texture_count = ri.loaded_texture_count;

    ri.loaded_texture_count += RI_number_of_requested_textures;

    ri.loaded_textures = realloc(ri.loaded_textures, sizeof(RI_texture) * ri.loaded_texture_count);
    
    for (int i = 0; i < RI_number_of_requested_textures; i++){
        RI_texture new_texture = {0};
        
        char *current_texture_filename = texture_creation_data[i].filename;

        unsigned char* temp_texture = stbi_load(current_texture_filename, &new_texture.resolution.x, &new_texture.resolution.y, NULL, 4);
        
        if(stbi_failure_reason()){
            new_texture.resolution.x = 1;
            new_texture.resolution.y = 1;
            
            new_texture.image_buffer[0] = 255;
            new_texture.image_buffer[1] = 0;
            new_texture.image_buffer[2] = 255;
            new_texture.image_buffer[3] = 128;
        }
        else {
            new_texture.image_buffer = malloc(sizeof(uint32_t) * 4 * new_texture.resolution.x * new_texture.resolution.y);

            for (int i = 0; i < new_texture.resolution.x * new_texture.resolution.y * 4; i++){
                new_texture.image_buffer[i] = temp_texture[i];
            }
        }

        ri.loaded_textures[previous_loaded_texture_count + i] = new_texture;

        stbi_image_free(temp_texture);
    }

    return ri.loaded_textures;
}

RI_mesh* RI_request_meshes(int RI_number_of_requested_meshes, char **filenames){
    int meshes_already_loaded_count = ri.loaded_mesh_count;
    
    ri.loaded_mesh_count += RI_number_of_requested_meshes;

    ri.loaded_meshes = realloc(ri.loaded_meshes, sizeof(RI_mesh) * ri.loaded_mesh_count);

    for (int i = 0; i < RI_number_of_requested_meshes; i++){
        RI_mesh new_mesh_data_struct = {0};

        FILE *file = fopen(filenames[i], "r");

        if (!file){
            debug("Error! File \"%s\" not found", filenames[i]);
            RI_stop(1);
        }
        
        char line[512];
        
        while (fgets(line, sizeof(line), file)) {
            if (line[0] == 'f' && line[1] == ' ') { // face
                new_mesh_data_struct.face_count++;
            }
            else if (line[0] == 'v' && line[1] == ' ') { // vertex
                new_mesh_data_struct.vertex_count++;
            }
            else if (line[0] == 'v' && line[1] == 'n') { // normal
                new_mesh_data_struct.normal_count++;
            }
            else if (line[0] == 'v' && line[1] == 't') { // UV
                new_mesh_data_struct.uv_count++;
            }
        }
        
        fclose(file);
        
        new_mesh_data_struct.faces = malloc(sizeof(RI_face) * new_mesh_data_struct.face_count);
        new_mesh_data_struct.vertecies = malloc(sizeof(RI_vertex) * new_mesh_data_struct.vertex_count);

        RI_vector_3f *normals = malloc(sizeof(RI_vector_3f) * new_mesh_data_struct.normal_count);
        RI_vector_2f *uvs = malloc(sizeof(RI_vector_2f) * new_mesh_data_struct.uv_count);

        FILE *file_again = fopen(filenames[i], "r");

        int current_face_index = 0;
        int current_vertex_index = 0;
        int current_normal_index = 0;
        int current_uv_index = 0;

        int has_normals, has_uvs;
        has_normals = has_uvs = 0;

        while (fgets(line, sizeof(line), file_again)) {
            if (line[0] == 'f' && line[1] == ' ') {
                int vertex_0_index, vertex_1_index, vertex_2_index, normal_0_index, normal_1_index, normal_2_index, uv_0_index, uv_1_index, uv_2_index;

                int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d/", 
                    &vertex_0_index, &uv_0_index, &normal_0_index, 
                    &vertex_1_index, &uv_1_index, &normal_1_index, 
                    &vertex_2_index, &uv_2_index, &normal_2_index);
    
                if (matches != 9){
                    vertex_0_index = -1;
                    vertex_1_index = -1;
                    vertex_2_index = -1;
                    
                    normal_0_index = -1;
                    normal_1_index = -1;
                    normal_2_index = -1;
                    
                    uv_0_index = -1;
                    uv_1_index = -1;
                    uv_2_index = -1;
    
                    if (strchr(line, '/')){
                        sscanf(line, "f %d//%d %d//%d %d//%d", 
                            &vertex_0_index, &normal_0_index, 
                            &vertex_1_index, &normal_1_index, 
                            &vertex_2_index, &normal_2_index);
                    
                        has_normals = 1;
                    }
                    else {
                        sscanf(line, "f %d %d %d", 
                            &vertex_0_index, 
                            &vertex_1_index, 
                            &vertex_2_index);
                    }
                }
                else {
                    has_normals = has_uvs = 1;
                }

                new_mesh_data_struct.faces[current_face_index].vertex_0 = &new_mesh_data_struct.vertecies[vertex_0_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_1 = &new_mesh_data_struct.vertecies[vertex_1_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_2 = &new_mesh_data_struct.vertecies[vertex_2_index - 1];

                new_mesh_data_struct.faces[current_face_index].vertex_0->original_normal = normals[normal_0_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_1->original_normal = normals[normal_1_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_2->original_normal = normals[normal_2_index - 1];

                new_mesh_data_struct.faces[current_face_index].vertex_0->uv = uvs[uv_0_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_1->uv = uvs[uv_1_index - 1];
                new_mesh_data_struct.faces[current_face_index].vertex_2->uv = uvs[uv_2_index - 1];

                ++current_face_index;
            }
            else if (line[0] == 'v' && line[1] == ' ') {
                float x, y, z;
                
                sscanf(line, "v %f %f %f", &x, &y, &z);

                new_mesh_data_struct.vertecies[current_vertex_index].original_position.x = x;
                new_mesh_data_struct.vertecies[current_vertex_index].original_position.y = y;
                new_mesh_data_struct.vertecies[current_vertex_index].original_position.z = z;

                ++current_vertex_index;
            } 
            else if (line[0] == 'v' && line[1] == 'n') {
                float x, y, z;
                
                sscanf(line, "vn %f %f %f", &x, &y, &z);

                normals[current_normal_index].x = x;
                normals[current_normal_index].y = y;
                normals[current_normal_index].z = z;

                ++current_normal_index;
            }
            else if (line[0] == 'v' && line[1] == 't') {
                float x, y, z;

                sscanf(line, "vt %f %f %f", &x, &y, &z);

                uvs[current_uv_index].x = x;
                uvs[current_uv_index].y = y;
                // UVS are almost always 2D so we don't need Z this (the type itself is a vector 2f, not 3f) 

                ++current_uv_index;
            } 
        }

        free(normals);
        free(uvs);

        char* loading_mesh_notice_string;

        if (has_normals && !has_uvs) loading_mesh_notice_string = "normals";
        else if (!has_normals && has_uvs) loading_mesh_notice_string = "UVs";
        else if (!has_normals && !has_uvs) loading_mesh_notice_string = "normals and UVs";
        
        if (!has_normals || !has_uvs) debug("Notice! Mesh \"%s\" is missing %s", filenames[i], loading_mesh_notice_string);
        
        // fclose(file_again);

        ri.loaded_meshes[meshes_already_loaded_count + i] = new_mesh_data_struct;   

        debug("Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", filenames[i], current_face_index, current_vertex_index, current_normal_index, current_uv_index); 
    }

    return ri.loaded_meshes;
}

void quaternion_rotation(RI_vector_3f *position, RI_vector_4f rotation){
    return;
}

void color_pixel(int x, int y, uint32_t color){
    ri.frame_buffer[(x + ri.window_height / 2) * ri.window_width + (y + ri.window_width / 2)] = color;
}

int RI_render(RI_scene *scene){
    // do rendering stuff
    if (ri.running){
        float horizontal_fov_factor = ri.window_width / tanf(0.5 * ri.FOV);
        float vertical_fov_factor = ri.window_height / tanf(0.5 * ri.FOV);

        for (int actor_index = 0; actor_index < scene->actor_count; ++actor_index){
            RI_actor *current_actor = scene->actors[actor_index];

            for (int polygon_index = 0; polygon_index < current_actor->mesh_reference->face_count; ++polygon_index){
                current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position = current_actor->mesh_reference->faces[polygon_index].vertex_0->original_position;
                current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position = current_actor->mesh_reference->faces[polygon_index].vertex_1->original_position;
                current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position = current_actor->mesh_reference->faces[polygon_index].vertex_2->original_position;

                quaternion_rotation(&current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position, current_actor->transform.rotation);
                quaternion_rotation(&current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position, current_actor->transform.rotation);
                quaternion_rotation(&current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position, current_actor->transform.rotation);

                vector_3f_hadamard(&current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position, current_actor->transform.scale);
                vector_3f_hadamard(&current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position, current_actor->transform.scale);
                vector_3f_hadamard(&current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position, current_actor->transform.scale);
            
                vector_3f_element_wise_add(&current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position, current_actor->transform.position);
                vector_3f_element_wise_add(&current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position, current_actor->transform.position);
                vector_3f_element_wise_add(&current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position, current_actor->transform.position);
            
                current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.x = current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.x / current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.z * horizontal_fov_factor;
                current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.y = current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.y / current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position.z * vertical_fov_factor;
                
                current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.x = current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.x / current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.z * horizontal_fov_factor;
                current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.y = current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.y / current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position.z * vertical_fov_factor;

                current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.x = current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.x / current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.z * horizontal_fov_factor;
                current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.y = current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.y / current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position.z * vertical_fov_factor;
            }
        }

        for (int pixel_index = 0; pixel_index < ri.window_width * ri.window_height; ++pixel_index){
            ri.frame_buffer[pixel_index] = 0x0;
            ri.z_buffer[pixel_index] = 99999;
        }

        for (int pixel_y_index = -ri.window_height / 2; pixel_y_index < ri.window_height / 2; ++pixel_y_index){
            for (int pixel_x_index = -ri.window_width / 2; pixel_x_index < ri.window_height / 2; ++pixel_x_index){
                for (int actor_index = 0; actor_index < scene->actor_count; ++actor_index){
                    RI_actor *current_actor = scene->actors[actor_index];

                    for (int polygon_index = 0; polygon_index < current_actor->mesh_reference->face_count; ++polygon_index){
                        RI_vector_3f *pos_0 = &current_actor->mesh_reference->faces[polygon_index].vertex_0->transformed_position;
                        RI_vector_3f *pos_1 = &current_actor->mesh_reference->faces[polygon_index].vertex_1->transformed_position;
                        RI_vector_3f *pos_2 = &current_actor->mesh_reference->faces[polygon_index].vertex_2->transformed_position;
                        
                        int vertex_0_out_of_bounds = pos_0->x < 0 || pos_0->x >= ri.window_width || pos_0->y < 0 || pos_0->y >= ri.window_height;
                        int vertex_1_out_of_bounds = pos_1->x < 0 || pos_1->x >= ri.window_width || pos_1->y < 0 || pos_1->y >= ri.window_height;
                        int vertex_2_out_of_bounds = pos_2->x < 0 || pos_2->x >= ri.window_width || pos_2->y < 0 || pos_2->y >= ri.window_height;
                        
                        if (vertex_0_out_of_bounds && vertex_1_out_of_bounds && vertex_2_out_of_bounds){
                            // continue;
                        }
                        
                        float denominator, w0, w1, w2;

                        denominator = (pos_1->y - pos_2->y) * (pos_0->x - pos_2->x) + (pos_2->x - pos_1->x) * (pos_0->y - pos_2->y);
                        w0 = ((pos_1->y - pos_2->y) * (pixel_x_index - pos_2->x) + (pos_2->x - pos_1->x) * (pixel_y_index - pos_2->y)) / denominator;
                        w1 = ((pos_2->y - pos_0->y) * (pixel_x_index - pos_0->x) + (pos_0->x - pos_2->x) * (pixel_y_index - pos_0->y)) / denominator; 
                        w2 = 1.0 - w0 - w1; 

                        float w_over_z = (w0 / pos_0->z + w1 / pos_1->z + w2 / pos_2->z); 
                        float interpolated_z = 1.0 / w_over_z;

                        if (!(w0 >= 0 && w1 >= 0 && w2 >= 0)){
                            continue;
                        }
                        
                        if (interpolated_z >= ri.z_buffer[(pixel_x_index + ri.window_height / 2) * ri.window_width + (pixel_y_index + ri.window_width / 2)]){
                            continue;
                        }   

                        ri.z_buffer[(pixel_x_index + ri.window_height / 2) * ri.window_width + (pixel_y_index + ri.window_width / 2)] = interpolated_z;
                        
                        color_pixel(pixel_y_index, pixel_x_index, 0x01010101 * polygon_index);
                    }
                }
            }
        }

        SDL_UpdateTexture(ri.texture, NULL, ri.frame_buffer, ri.window_width * sizeof(uint32_t));

        SDL_RenderClear(ri.renderer);
        SDL_RenderCopy(ri.renderer, ri.texture, NULL, NULL);
    
        SDL_RenderPresent(ri.renderer);
    }
    else{
        RI_stop(0);
    }

    // handle SDL events
    while (SDL_PollEvent(&ri.event)){
        switch (ri.event.type){
            case SDL_QUIT:
                ri.running = 0;
        }
    }

    ++ri.frame;

    return 0;
}

int opencl_init(){
    return 0;
}

int sdl_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.window_width = RI_window_width;
    ri.window_height = RI_window_height;
    ri.window_title = RI_window_title;

    ri.frame_buffer = malloc(sizeof(uint32_t) * ri.window_width * ri.window_height);
    ri.z_buffer = malloc(sizeof(float) * ri.window_width * ri.window_height);

    SDL_Init(SDL_INIT_VIDEO);

    ri.window = SDL_CreateWindow(RI_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ri.window_width, ri.window_height, SDL_WINDOW_OPENGL);

    ri.renderer = SDL_CreateRenderer(ri.window, -1, SDL_RENDERER_ACCELERATED);

    ri.texture = SDL_CreateTexture(ri.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ri.window_width, ri.window_height);

    return 0;
}

int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.running = 1;

    opencl_init();

    sdl_init(RI_window_width, RI_window_height, RI_window_title);

    ri.loaded_mesh_count = 0;
    ri.loaded_texture_count = 0;
    ri.actor_count = 0;

    ri.prefix = "[RasterIver] ";

    return 0;
}

int RI_stop(int result){
    debug("Stopping...");
    
    for (int mesh_index = 0; mesh_index < ri.loaded_mesh_count; ++mesh_index){
        free(ri.loaded_meshes[mesh_index].faces); // free face array
        free(ri.loaded_meshes[mesh_index].vertecies); // free vertex array
    }

    for (int texture_index = 0; texture_index < ri.loaded_texture_count; ++texture_index){
        free(ri.loaded_textures[texture_index].image_buffer);
    }

    exit(result);

    return 0;
}