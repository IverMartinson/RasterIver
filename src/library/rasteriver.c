#include <stdio.h>
#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#include "stdint.h"
#include <math.h>

RasterIver ri = {NULL};

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

RI_actor* RI_request_actors(int RI_number_of_requested_actors){
    int previous_actor_count = ri.actor_count;
    ri.actor_count += RI_number_of_requested_actors;
    
    ri.actors = realloc(ri.actors, sizeof(RI_actor) * ri.actor_count);

    for (int i = 0; i < RI_number_of_requested_actors; ++i){
        RI_actor new_actor = {0};

        new_actor.mesh_reference = NULL;
        new_actor.material_reference = NULL;

        ri.actors[i + previous_actor_count] = new_actor;
    }

    return ri.actors;
}

RI_material* RI_request_materials(int RI_number_of_requested_materials){
    ri.material_count += RI_number_of_requested_materials;

    ri.materials = realloc(ri.materials, sizeof(RI_material) * ri.material_count);

    return ri.materials;
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
            new_texture = ri.error_texture;
        }
        else {
            new_texture.image_buffer = malloc(sizeof(uint32_t) * new_texture.resolution.x * new_texture.resolution.y);

            for (int i = 0; i < new_texture.resolution.x * new_texture.resolution.y; ++i){
                unsigned char r = temp_texture[i * 4];
                unsigned char g = temp_texture[i * 4 + 1];
                unsigned char b = temp_texture[i * 4 + 2];
                unsigned char a = temp_texture[i * 4 + 3];
                
                new_texture.image_buffer[i] = (a << 24 | r << 16 | g << 8 | b);
            }
        }

        ri.loaded_textures[previous_loaded_texture_count + i] = new_texture;

        stbi_image_free(temp_texture);
    }

    return ri.loaded_textures;
}

RI_mesh* RI_request_meshes(int RI_number_of_requested_meshes, char **filenames, int RI_return_just_mesh){
    int meshes_already_loaded_count = ri.loaded_mesh_count;
    
    RI_mesh* mesh;
    
    if (!RI_return_just_mesh) {
        ri.loaded_mesh_count += RI_number_of_requested_meshes;

        ri.loaded_meshes = realloc(ri.loaded_meshes, sizeof(RI_mesh) * ri.loaded_mesh_count);
    }
    else {
        mesh = malloc(sizeof(RI_mesh));
    }

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
        new_mesh_data_struct.vertex_positions = malloc(sizeof(RI_vector_3f) * new_mesh_data_struct.vertex_count);
        new_mesh_data_struct.normals = malloc(sizeof(RI_vector_3f) * new_mesh_data_struct.normal_count);
        new_mesh_data_struct.uvs = malloc(sizeof(RI_vector_2f) * new_mesh_data_struct.uv_count);

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

                new_mesh_data_struct.faces[current_face_index].position_0_index = vertex_0_index - 1;
                new_mesh_data_struct.faces[current_face_index].position_1_index = vertex_1_index - 1;
                new_mesh_data_struct.faces[current_face_index].position_2_index = vertex_2_index - 1;

                new_mesh_data_struct.faces[current_face_index].normal_0_index = normal_0_index - 1;
                new_mesh_data_struct.faces[current_face_index].normal_1_index = normal_1_index - 1;
                new_mesh_data_struct.faces[current_face_index].normal_2_index = normal_2_index - 1;
                
                new_mesh_data_struct.faces[current_face_index].uv_0_index = uv_0_index - 1;
                new_mesh_data_struct.faces[current_face_index].uv_1_index = uv_1_index - 1;
                new_mesh_data_struct.faces[current_face_index].uv_2_index = uv_2_index - 1;

                ++current_face_index;
            }
            else if (line[0] == 'v' && line[1] == ' ') {
                float x, y, z;
                
                sscanf(line, "v %f %f %f", &x, &y, &z);

                new_mesh_data_struct.vertex_positions[current_vertex_index].x = x;
                new_mesh_data_struct.vertex_positions[current_vertex_index].y = y;
                new_mesh_data_struct.vertex_positions[current_vertex_index].z = z;

                ++current_vertex_index;
            } 
            else if (line[0] == 'v' && line[1] == 'n') {
                float x, y, z;
                
                sscanf(line, "vn %f %f %f", &x, &y, &z);

                new_mesh_data_struct.normals[current_normal_index].x = x;
                new_mesh_data_struct.normals[current_normal_index].y = y;
                new_mesh_data_struct.normals[current_normal_index].z = z;

                ++current_normal_index;
            }
            else if (line[0] == 'v' && line[1] == 't') {
                float x, y, z;

                sscanf(line, "vt %f %f %f", &x, &y, &z);

                new_mesh_data_struct.uvs[current_uv_index].x = x;
                new_mesh_data_struct.uvs[current_uv_index].y = y;
                // UVS are almost always 2D so we don't need Z (the type itself is a vector 2f, not 3f) 

                ++current_uv_index;
            } 
        }

        char* loading_mesh_notice_string;

        if (has_normals && !has_uvs) loading_mesh_notice_string = "normals";
        else if (!has_normals && has_uvs) loading_mesh_notice_string = "UVs";
        else if (!has_normals && !has_uvs) loading_mesh_notice_string = "normals and UVs";
        
        if (!has_normals || !has_uvs) debug("Notice! Mesh \"%s\" is missing %s", filenames[i], loading_mesh_notice_string);
        
        // fclose(file_again);

        if (!RI_return_just_mesh) {
            ri.loaded_meshes[meshes_already_loaded_count + i] = new_mesh_data_struct;   

            debug("Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", filenames[i], current_face_index, current_vertex_index, current_normal_index, current_uv_index); 
        }
        else {
            *mesh = new_mesh_data_struct;
        }
    }

    if (!RI_return_just_mesh) return ri.loaded_meshes;
    else return mesh;
}

void quaternion_rotate(RI_vector_3f *position, RI_vector_4f rotation){
    RI_vector_4f pos_quat = {0, position->x, position->y, position->z};

    RI_vector_4f rotation_conjugation = rotation;
    quaternion_conjugate(&rotation_conjugation);

    quaternion_multiply(&rotation, pos_quat);

    quaternion_multiply(&rotation, rotation_conjugation);

    *position = (RI_vector_3f){rotation.x, rotation.y, rotation.z};
}

void RI_euler_rotation_to_quaternion(RI_vector_4f *quaternion, RI_vector_3f euler_rotation){
    float cx = cosf(euler_rotation.x * 0.5f);
    float sx = sinf(euler_rotation.x * 0.5f);
    float cy = cosf(euler_rotation.y * 0.5f);
    float sy = sinf(euler_rotation.y * 0.5f);
    float cz = cosf(euler_rotation.z * 0.5f);
    float sz = sinf(euler_rotation.z * 0.5f);

    quaternion->w = cx * cy * cz + sx * sy * sz;
    quaternion->x = sx * cy * cz - cx * sy * sz;
    quaternion->y = cx * sy * cz + sx * cy * sz;
    quaternion->z = cx * cy * sz - sx * sy * cz;
}

int RI_render(RI_scene *scene, RI_texture *target_texture){
    // do rendering stuff
    if (ri.running){
        float horizontal_fov_factor = target_texture->resolution.x / tanf(0.5 * scene->FOV);
        float vertical_fov_factor = target_texture->resolution.y / tanf(0.5 * scene->FOV);
        
        for (int pixel_index = 0; pixel_index < target_texture->resolution.x * target_texture->resolution.y; ++pixel_index){
            target_texture->image_buffer[pixel_index] = 0x0;
            ri.z_buffer[pixel_index] = 99999;
        }

        for (int pixel_y_index = -target_texture->resolution.y / 2; pixel_y_index < target_texture->resolution.y / 2; ++pixel_y_index){
            for (int pixel_x_index = -target_texture->resolution.x / 2; pixel_x_index < target_texture->resolution.x / 2; ++pixel_x_index){
                for (int actor_index = 0; actor_index < scene->actor_count; ++actor_index){
                    RI_actor *current_actor = scene->actors[actor_index];

                    if (!current_actor->transformed_vertex_positions){
                        current_actor->transformed_vertex_positions = malloc(sizeof(RI_vector_3f) * current_actor->mesh_reference->vertex_count);
                    }

                    for (int polygon_index = 0; polygon_index < current_actor->mesh_reference->face_count; ++polygon_index){
                        int vert_pos_0_index = current_actor->mesh_reference->faces[polygon_index].position_0_index;
                        int vert_pos_1_index = current_actor->mesh_reference->faces[polygon_index].position_1_index;
                        int vert_pos_2_index = current_actor->mesh_reference->faces[polygon_index].position_2_index;
                        
                        int normal_0_index = current_actor->mesh_reference->faces[polygon_index].normal_0_index;
                        int normal_1_index = current_actor->mesh_reference->faces[polygon_index].normal_1_index;
                        int normal_2_index = current_actor->mesh_reference->faces[polygon_index].normal_2_index;

                        int uv_0_index = current_actor->mesh_reference->faces[polygon_index].uv_0_index;
                        int uv_1_index = current_actor->mesh_reference->faces[polygon_index].uv_1_index;
                        int uv_2_index = current_actor->mesh_reference->faces[polygon_index].uv_2_index;

                        current_actor->transformed_vertex_positions[vert_pos_0_index] = current_actor->mesh_reference->vertex_positions[vert_pos_0_index];
                        current_actor->transformed_vertex_positions[vert_pos_1_index] = current_actor->mesh_reference->vertex_positions[vert_pos_1_index];
                        current_actor->transformed_vertex_positions[vert_pos_2_index] = current_actor->mesh_reference->vertex_positions[vert_pos_2_index];
                        
                        // scale
                        vector_3f_hadamard(&current_actor->transformed_vertex_positions[vert_pos_0_index], current_actor->transform.scale);
                        vector_3f_hadamard(&current_actor->transformed_vertex_positions[vert_pos_1_index], current_actor->transform.scale);
                        vector_3f_hadamard(&current_actor->transformed_vertex_positions[vert_pos_2_index], current_actor->transform.scale);

                        // combine camera and object rotation
                        RI_vector_4f combined_rotation = current_actor->transform.rotation;
                        RI_vector_4f camera_rotation = scene->camera_rotation;
                    
                        quaternion_conjugate(&camera_rotation);
                        
                        quaternion_multiply(&combined_rotation, camera_rotation);

                        // rotate
                        quaternion_rotate(&current_actor->transformed_vertex_positions[vert_pos_0_index], combined_rotation);
                        quaternion_rotate(&current_actor->transformed_vertex_positions[vert_pos_1_index], combined_rotation);
                        quaternion_rotate(&current_actor->transformed_vertex_positions[vert_pos_2_index], combined_rotation);
                        
                        // object position
                        vector_3f_element_wise_add(&current_actor->transformed_vertex_positions[vert_pos_0_index], current_actor->transform.position);
                        vector_3f_element_wise_add(&current_actor->transformed_vertex_positions[vert_pos_1_index], current_actor->transform.position);
                        vector_3f_element_wise_add(&current_actor->transformed_vertex_positions[vert_pos_2_index], current_actor->transform.position);
                    
                        // camera position
                        vector_3f_element_wise_subtract(&current_actor->transformed_vertex_positions[vert_pos_0_index], scene->camera_position);
                        vector_3f_element_wise_subtract(&current_actor->transformed_vertex_positions[vert_pos_1_index], scene->camera_position);
                        vector_3f_element_wise_subtract(&current_actor->transformed_vertex_positions[vert_pos_2_index], scene->camera_position);

                        current_actor->transformed_vertex_positions[vert_pos_0_index].x = current_actor->transformed_vertex_positions[vert_pos_0_index].x / current_actor->transformed_vertex_positions[vert_pos_0_index].z * horizontal_fov_factor;
                        current_actor->transformed_vertex_positions[vert_pos_0_index].y = current_actor->transformed_vertex_positions[vert_pos_0_index].y / current_actor->transformed_vertex_positions[vert_pos_0_index].z * vertical_fov_factor;
                        
                        current_actor->transformed_vertex_positions[vert_pos_1_index].x = current_actor->transformed_vertex_positions[vert_pos_1_index].x / current_actor->transformed_vertex_positions[vert_pos_1_index].z * horizontal_fov_factor;
                        current_actor->transformed_vertex_positions[vert_pos_1_index].y = current_actor->transformed_vertex_positions[vert_pos_1_index].y / current_actor->transformed_vertex_positions[vert_pos_1_index].z * vertical_fov_factor;

                        current_actor->transformed_vertex_positions[vert_pos_2_index].x = current_actor->transformed_vertex_positions[vert_pos_2_index].x / current_actor->transformed_vertex_positions[vert_pos_2_index].z * horizontal_fov_factor;
                        current_actor->transformed_vertex_positions[vert_pos_2_index].y = current_actor->transformed_vertex_positions[vert_pos_2_index].y / current_actor->transformed_vertex_positions[vert_pos_2_index].z * vertical_fov_factor;

                        RI_vector_3f *pos_0 = &current_actor->transformed_vertex_positions[vert_pos_0_index];
                        RI_vector_3f *pos_1 = &current_actor->transformed_vertex_positions[vert_pos_1_index];
                        RI_vector_3f *pos_2 = &current_actor->transformed_vertex_positions[vert_pos_2_index];
                        
                        RI_material *mat = current_actor->material_reference;

                        RI_vector_2f *uv_0;
                        RI_vector_2f *uv_1;
                        RI_vector_2f *uv_2;

                        if (mat == NULL){
                            mat = &ri.error_material;
                        }

                        if(mat->flags & RI_MATERIAL_HAS_TEXTURE && mat->texture_reference == NULL){
                            mat->texture_reference = &ri.error_texture;
                        }

                        if(mat->flags & RI_MATERIAL_HAS_BUMP_MAP && mat->bump_map_reference == NULL){
                            mat->bump_map_reference = &ri.error_bump_map;
                        }

                        if(mat->flags & RI_MATERIAL_HAS_NORMAL_MAP && mat->normal_map_reference == NULL){
                            mat->normal_map_reference = &ri.error_normal_map;
                        }

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

                        if (!(w0 >= 0 && w1 >= 0 && w2 >= 0) || (mat->flags & RI_MATERIAL_WIREFRAME && (w0 >= mat->wireframe_width && w1 >= mat->wireframe_width && w2 >= mat->wireframe_width))){
                            continue;
                        }
                        
                        if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_TEST) && interpolated_z >= ri.z_buffer[(pixel_x_index + target_texture->resolution.y / 2) * target_texture->resolution.x + (pixel_y_index + target_texture->resolution.x / 2)]){
                            continue;
                        }   

                        if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_WRITE)){
                            ri.z_buffer[(pixel_x_index + target_texture->resolution.y / 2) * target_texture->resolution.x + (pixel_y_index + target_texture->resolution.x / 2)] = interpolated_z;
                        }
                        
                        uint32_t pixel_color = 0xFF000000;
                        
                        if (mat->flags & RI_MATERIAL_HAS_TEXTURE){
                            uv_0 = &current_actor->mesh_reference->uvs[uv_0_index];
                            uv_1 = &current_actor->mesh_reference->uvs[uv_1_index];
                            uv_2 = &current_actor->mesh_reference->uvs[uv_2_index];

                            double ux = (w0 * (uv_0->x / pos_0->z) + w1 * (uv_1->x / pos_1->z) + w2 * (uv_2->x / pos_2->z)) / w_over_z;
                            double uy = (w0 * (uv_0->y / pos_0->z) + w1 * (uv_1->y / pos_1->z) + w2 * (uv_2->y / pos_2->z)) / w_over_z;                
                        
                            RI_vector_2 texel_position = {mat->texture_reference->resolution.x * ux, mat->texture_reference->resolution.y * uy};
                            
                            pixel_color = mat->texture_reference->image_buffer[texel_position.y * mat->texture_reference->resolution.x + texel_position.x];
                        }
                        else { // must be only an albedo
                            if (mat->albedo) pixel_color = mat->albedo;
                            else pixel_color = 0xFFFF77FF;
                        }

                        int x = pixel_x_index;
                        int y = pixel_y_index;
                        
                        x += target_texture->resolution.x / 2;
                        y += target_texture->resolution.y / 2;

                        // x = target_texture->resolution.x - 1 - x;
                        // y = target_texture->resolution.y - 1 - y;

                        if (x >= 0 && y >= 0 && x < target_texture->resolution.x && y < target_texture->resolution.y){
                            target_texture->image_buffer[y * target_texture->resolution.y + x] = pixel_color;
                        }   
                    }
                }
            }
        }

        SDL_UpdateTexture(ri.texture, NULL, ri.frame_buffer->image_buffer, ri.window_width * sizeof(uint32_t));

        SDL_RenderClear(ri.renderer);
        SDL_RenderCopyEx(ri.renderer, ri.texture, NULL, NULL, 0, NULL, SDL_FLIP_VERTICAL);
    
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

    ri.frame_buffer = malloc(sizeof(RI_texture));

    ri.frame_buffer->image_buffer = malloc(sizeof(uint32_t) * ri.window_width * ri.window_height);
    ri.frame_buffer->resolution = (RI_vector_2){ri.window_width, ri.window_height};
    
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

    char **error_cube_file = malloc(sizeof(char *));
    error_cube_file[0] = "objects/unit_cube.obj";

    RI_mesh* error_mesh = RI_request_meshes(1, error_cube_file, 1);

    ri.error_mesh = *error_mesh;

    free(error_mesh);
    free(error_cube_file);

    ri.error_texture.image_buffer = malloc(sizeof(uint32_t));

    ri.error_texture.image_buffer[0] = 0xFFFF00FF;
    ri.error_texture.resolution = (RI_vector_2){1, 1};

    ri.error_material.texture_reference = &ri.error_texture;
    ri.error_material.albedo = 0xFF5522CC;
    ri.error_material.flags = RI_MATERIAL_UNLIT | RI_MATERIAL_DONT_DEPTH_TEST | RI_MATERIAL_DONT_RECEIVE_SHADOW | RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;

    return 0;
}

int RI_stop(int result){
    debug("Stopping...");
    
    for (int mesh_index = 0; mesh_index < ri.loaded_mesh_count; ++mesh_index){
        free(ri.loaded_meshes[mesh_index].faces); // free face array
    }

    for (int texture_index = 0; texture_index < ri.loaded_texture_count; ++texture_index){
        free(ri.loaded_textures[texture_index].image_buffer);
    }

    exit(result);

    return 0;
}