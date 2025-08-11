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

#define RI_realloc(__ptr, __size) written_RI_realloc(__ptr, __size, __func__, __LINE__)
#define RI_malloc(__size) written_RI_malloc(__size, __func__, __LINE__)
#define RI_calloc(__nmemb, __size) written_RI_calloc(__nmemb, __size, __func__, __LINE__)
#define RI_free(__ptr) written_RI_free(__ptr, __func__, __LINE__)

void* written_RI_realloc(void *__ptr, size_t __size, const char *caller, int line){
    void *pointer = realloc(__ptr, __size);

    if (ri.debug_memory) {
        int current_allocation_index = 0;
        int checking = 1;

        while (checking){
            if (!ri.allocation_table[current_allocation_index].reallocated_free && ri.allocation_table[current_allocation_index].pointer == __ptr){
                ri.allocation_table[current_allocation_index].reallocated_free = 1;
                
                checking = 0;
            }

            current_allocation_index++;
            
            if (current_allocation_index >= ri.allocation_search_limit){
                checking = 0;
            }
        }

        debug("[Memory Manager] Allocated (realloc) %zu bytes (func \"%s\":%d)", __size, caller, line);

        if (ri.current_allocation_index >= ri.allocation_table_length){
            ri.allocation_table_length += 50;
            ri.allocation_search_limit += 50;
            
            ri.allocation_table = RI_realloc(ri.allocation_table, sizeof(RI_memory_allocation) * ri.allocation_table_length);
        }

        ri.allocation_table[ri.current_allocation_index].allocated = 1;
        ri.allocation_table[ri.current_allocation_index].reallocated_alloc = 1;
        ri.allocation_table[ri.current_allocation_index].reallocated_free = 0;
        ri.allocation_table[ri.current_allocation_index].freed = 0;
        ri.allocation_table[ri.current_allocation_index].line = line;
        ri.allocation_table[ri.current_allocation_index].pointer = pointer;        
        ri.allocation_table[ri.current_allocation_index].size = __size;

        ri.current_allocation_index++;
    }

    return pointer;
}

void* written_RI_malloc(size_t __size, const char *caller, int line){
    void *pointer = malloc(__size);
    
    if (ri.debug_memory) {
        debug("[Memory Manager] Allocated (malloc) %zu bytes (func \"%s\":%d)", __size, caller, line);

        if (ri.current_allocation_index >= ri.allocation_table_length){
            ri.allocation_table_length += 50;
            ri.allocation_search_limit += 50;
            
            ri.allocation_table = RI_realloc(ri.allocation_table, sizeof(RI_memory_allocation) * ri.allocation_table_length);
        }

        ri.allocation_table[ri.current_allocation_index].allocated = 1;
        ri.allocation_table[ri.current_allocation_index].reallocated_free = 0;
        ri.allocation_table[ri.current_allocation_index].reallocated_alloc = 0;
        ri.allocation_table[ri.current_allocation_index].freed = 0;
        ri.allocation_table[ri.current_allocation_index].line = line;
        ri.allocation_table[ri.current_allocation_index].pointer = pointer;        
        ri.allocation_table[ri.current_allocation_index].size = __size;

        ri.current_allocation_index++;
    }

    return pointer;
}

void* written_RI_calloc(size_t __nmemb, size_t __size, const char *caller, int line){
    void *pointer = calloc(__nmemb, __size);
    
    if (ri.debug_memory) {
        debug("[Memory Manager] Allocated (calloc) %zu bytes (func \"%s\":%d)", __size * __nmemb, caller, line);

        if (ri.current_allocation_index >= ri.allocation_table_length){
            ri.allocation_table_length += 50;
            ri.allocation_search_limit += 50;
            
            ri.allocation_table = RI_realloc(ri.allocation_table, sizeof(RI_memory_allocation) * ri.allocation_table_length);
        }

        ri.allocation_table[ri.current_allocation_index].allocated = 1;
        ri.allocation_table[ri.current_allocation_index].reallocated_free = 0;
        ri.allocation_table[ri.current_allocation_index].reallocated_alloc = 0;
        ri.allocation_table[ri.current_allocation_index].freed = 0;
        ri.allocation_table[ri.current_allocation_index].line = line;
        ri.allocation_table[ri.current_allocation_index].pointer = pointer;        
        ri.allocation_table[ri.current_allocation_index].size = __size * __nmemb;
            
        ri.current_allocation_index++;
    }

    return pointer;
}

void written_RI_free(void *__ptr, const char *caller, int line){
    if (ri.debug_memory) {
        size_t size = 0;
        
        int current_allocation_index = 0;
        int checking = 1;
        
        while (checking){
            if (!ri.allocation_table[current_allocation_index].reallocated_free && ri.allocation_table[current_allocation_index].pointer == __ptr){
                size = ri.allocation_table[current_allocation_index].size;
                ri.allocation_table[current_allocation_index].freed = 1;
                
                checking = 0;
            }
            
            current_allocation_index++;
            
            if (current_allocation_index >= ri.allocation_search_limit){
                checking = 0;
            }
        }
        
        debug("[Memory Manager] Freed %zu bytes (func \"%s\":%d)", size, caller, line);
    }
 
    free(__ptr);
}

int RI_add_actors_to_scene(int RI_number_of_actors_to_add_to_scene, RI_actor *actors, RI_scene *scene){
    int previous_actor_count = scene->actor_count;

    scene->actor_count += RI_number_of_actors_to_add_to_scene;

    scene->actors = RI_realloc(scene->actors, sizeof(RI_actor *) * scene->actor_count);

    for (int i = 0; i < RI_number_of_actors_to_add_to_scene; ++i){
        scene->actors[i + previous_actor_count] = &actors[i];
    }

    return 0;
}

RI_scene* RI_request_scenes(int RI_number_of_requested_scenes){
    int previous_scene_count = ri.scene_count;
    ri.scene_count += RI_number_of_requested_scenes;
    
    ri.scenes = RI_realloc(ri.scenes, sizeof(RI_scene) * ri.scene_count);

    for (int i = 0; i < RI_number_of_requested_scenes; ++i){
        RI_scene new_scene = {0};

        new_scene.actor_count = 0;
        new_scene.actors = NULL;
        new_scene.faces_to_render = NULL;
        
        ri.scenes[i + previous_scene_count] = new_scene;
    }

    return ri.scenes;
}

RI_actor* RI_request_actors(int RI_number_of_requested_actors){
    int previous_actor_count = ri.actor_count;
    ri.actor_count += RI_number_of_requested_actors;
    
    ri.actors = RI_realloc(ri.actors, sizeof(RI_actor) * ri.actor_count);

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

    ri.materials = RI_realloc(ri.materials, sizeof(RI_material) * ri.material_count);

    return ri.materials;
}

RI_texture* RI_request_textures(int RI_number_of_requested_textures, RI_texture_creation_data *texture_creation_data){
    int previous_loaded_texture_count = ri.loaded_texture_count;

    ri.loaded_texture_count += RI_number_of_requested_textures;

    ri.loaded_textures = RI_realloc(ri.loaded_textures, sizeof(RI_texture) * ri.loaded_texture_count);
    
    for (int i = 0; i < RI_number_of_requested_textures; i++){
        RI_texture new_texture = {0};
        
        char *current_texture_filename = texture_creation_data[i].filename;

        unsigned char* temp_texture = stbi_load(current_texture_filename, &new_texture.resolution.x, &new_texture.resolution.y, NULL, 4);
        
        if(stbi_failure_reason()){
            new_texture = ri.error_texture;
        }
        else {
            new_texture.image_buffer = RI_malloc(sizeof(uint32_t) * new_texture.resolution.x * new_texture.resolution.y);

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

        ri.loaded_meshes = RI_realloc(ri.loaded_meshes, sizeof(RI_mesh) * ri.loaded_mesh_count);
    }
    else {
        mesh = RI_malloc(sizeof(RI_mesh));
    }

    for (int i = 0; i < RI_number_of_requested_meshes; i++){
        RI_mesh new_mesh_data_struct = {0};

        FILE *file = fopen(filenames[i], "r");

        if (!file){
            debug("[Mesh Loader] Error! File \"%s\" not found", filenames[i]);
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
        
        new_mesh_data_struct.faces = RI_malloc(sizeof(RI_face) * new_mesh_data_struct.face_count);
        new_mesh_data_struct.vertex_positions = RI_malloc(sizeof(RI_vector_3f) * new_mesh_data_struct.vertex_count);
        new_mesh_data_struct.normals = RI_malloc(sizeof(RI_vector_3f) * new_mesh_data_struct.normal_count);
        new_mesh_data_struct.uvs = RI_malloc(sizeof(RI_vector_2f) * new_mesh_data_struct.uv_count);

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

                new_mesh_data_struct.faces[current_face_index].should_render = 1;

                ++current_face_index;
            }
            else if (line[0] == 'v' && line[1] == ' ') {
                double x, y, z;
                
                sscanf(line, "v %lf %lf %lf", &x, &y, &z);

                new_mesh_data_struct.vertex_positions[current_vertex_index].x = x;
                new_mesh_data_struct.vertex_positions[current_vertex_index].y = y;
                new_mesh_data_struct.vertex_positions[current_vertex_index].z = z;

                ++current_vertex_index;
            } 
            else if (line[0] == 'v' && line[1] == 'n') {
                double x, y, z;
                
                sscanf(line, "vn %lf %lf %lf", &x, &y, &z);

                new_mesh_data_struct.normals[current_normal_index].x = x;
                new_mesh_data_struct.normals[current_normal_index].y = y;
                new_mesh_data_struct.normals[current_normal_index].z = z;

                ++current_normal_index;
            }
            else if (line[0] == 'v' && line[1] == 't') {
                double x, y, z;

                sscanf(line, "vt %lf %lf %lf", &x, &y, &z);

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
        
        if (!has_normals || !has_uvs) debug("[Mesh Loader] Notice! Mesh \"%s\" is missing %s", filenames[i], loading_mesh_notice_string);
        
        new_mesh_data_struct.has_normals = has_normals;
        new_mesh_data_struct.has_uvs = has_uvs;

        // fclose(file_again);

        if (!RI_return_just_mesh) {
            ri.loaded_meshes[meshes_already_loaded_count + i] = new_mesh_data_struct;   

            debug("[Mesh Loader] Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", filenames[i], current_face_index, current_vertex_index, current_normal_index, current_uv_index); 
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
    double cx = cosf(euler_rotation.x * 0.5f);
    double sx = sinf(euler_rotation.x * 0.5f);
    double cy = cosf(euler_rotation.y * 0.5f);
    double sy = sinf(euler_rotation.y * 0.5f);
    double cz = cosf(euler_rotation.z * 0.5f);
    double sz = sinf(euler_rotation.z * 0.5f);

    quaternion->w = cx * cy * cz + sx * sy * sz;
    quaternion->x = sx * cy * cz - cx * sy * sz;
    quaternion->y = cx * sy * cz + sx * cy * sz;
    quaternion->z = cx * cy * sz - sx * sy * cz;
}

double mod(double a, double b){
    if(b < 0.0)
        return -mod(-a, -b);   
    double ret = fmod(a, b);
    if(ret < 0.0)
        ret+=b;
    return ret;
}

int RI_render(RI_scene *scene, RI_texture *target_texture){
    // do rendering stuff
    if (ri.running){
        double horizontal_fov_factor = target_texture->resolution.x / tanf(0.5 * scene->FOV);
        double vertical_fov_factor = target_texture->resolution.y / tanf(0.5 * scene->FOV);

        scene->min_clip = scene->minimum_clip_distance;

        if (!scene->faces_to_render){
            int total_faces = 0;
            
            for (int actor_index = 0; actor_index < scene->actor_count; ++actor_index){
                total_faces += scene->actors[actor_index]->mesh_reference->face_count;
            }

            scene->faces_to_render = RI_malloc(sizeof(RI_renderable_face) * total_faces * 2); // x2 because faces can be split
            scene->face_count = total_faces;
        }

        memset(scene->faces_to_render, 0, sizeof(RI_renderable_face) * scene->face_count * 2);

        int current_renderable_face_index = 0;
        int current_split_renderable_face_index = 0;

        for (int actor_index = 0; actor_index < scene->actor_count; ++actor_index){
            RI_actor *current_actor = scene->actors[actor_index];

            for (int face_index = 0; face_index < current_actor->mesh_reference->face_count; ++face_index){
                RI_face *cur_face = &current_actor->mesh_reference->faces[face_index];
                
                if (!cur_face->should_render){
                    continue;
                }

                int vert_pos_0_index = cur_face->position_0_index;
                int vert_pos_1_index = cur_face->position_1_index;
                int vert_pos_2_index = cur_face->position_2_index;
                
                int normal_0_index = cur_face->normal_0_index;
                int normal_1_index = cur_face->normal_1_index;
                int normal_2_index = cur_face->normal_2_index;

                int uv_0_index = cur_face->uv_0_index;
                int uv_1_index = cur_face->uv_1_index;
                int uv_2_index = cur_face->uv_2_index;

                RI_renderable_face *cur_r_face = &scene->faces_to_render[current_renderable_face_index];

                cur_r_face->parent_actor = current_actor;

                cur_r_face->material_reference = current_actor->material_reference;

                cur_r_face->position_0 = current_actor->mesh_reference->vertex_positions[vert_pos_0_index];
                cur_r_face->position_1 = current_actor->mesh_reference->vertex_positions[vert_pos_1_index];
                cur_r_face->position_2 = current_actor->mesh_reference->vertex_positions[vert_pos_2_index];

                if (current_actor->mesh_reference->has_uvs){
                    cur_r_face->uv_0 = current_actor->mesh_reference->uvs[uv_0_index];
                    cur_r_face->uv_1 = current_actor->mesh_reference->uvs[uv_1_index];
                    cur_r_face->uv_2 = current_actor->mesh_reference->uvs[uv_2_index];
                }

                // scale
                vector_3f_hadamard(&cur_r_face->position_0, current_actor->transform.scale);
                vector_3f_hadamard(&cur_r_face->position_1, current_actor->transform.scale);
                vector_3f_hadamard(&cur_r_face->position_2, current_actor->transform.scale);

                // actor rotation
                quaternion_rotate(&cur_r_face->position_0, current_actor->transform.rotation);
                quaternion_rotate(&cur_r_face->position_1, current_actor->transform.rotation);
                quaternion_rotate(&cur_r_face->position_2, current_actor->transform.rotation);
                
                // object position
                vector_3f_element_wise_add(&cur_r_face->position_0, current_actor->transform.position);
                vector_3f_element_wise_add(&cur_r_face->position_1, current_actor->transform.position);
                vector_3f_element_wise_add(&cur_r_face->position_2, current_actor->transform.position);
                
                // camera rotation
                vector_3f_element_wise_subtract(&cur_r_face->position_0, scene->camera_position);
                vector_3f_element_wise_subtract(&cur_r_face->position_1, scene->camera_position);
                vector_3f_element_wise_subtract(&cur_r_face->position_2, scene->camera_position);

                quaternion_rotate(&cur_r_face->position_0, scene->camera_rotation);
                quaternion_rotate(&cur_r_face->position_1, scene->camera_rotation);
                quaternion_rotate(&cur_r_face->position_2, scene->camera_rotation);
                
                // camera position
                // vector_3f_element_wise_subtract(&cur_r_face->position_0, scene->camera_position);
                // vector_3f_element_wise_subtract(&cur_r_face->position_1, scene->camera_position);
                // vector_3f_element_wise_subtract(&cur_r_face->position_2, scene->camera_position);

                RI_vector_3f *pos_0 = &cur_r_face->position_0;
                RI_vector_3f *pos_1 = &cur_r_face->position_1;
                RI_vector_3f *pos_2 = &cur_r_face->position_2;

                int is_0_clipped = pos_0->z < scene->min_clip;
                int is_1_clipped = pos_1->z < scene->min_clip;
                int is_2_clipped = pos_2->z < scene->min_clip;

                int clip_count = is_0_clipped + is_1_clipped + is_2_clipped;

                cur_r_face->should_render = 1;

                switch(clip_count){
                    case 3: // ignore polygon, it's behind the camera
                        continue;
                        break;
                    
                    case 2:{ // shrink poylgon
                        RI_vector_3f *unclipped_point, *point_a, *point_b;
                        RI_vector_3f *unclipped_normal, *normal_a, *normal_b;
                        RI_vector_2f *unclipped_uv, *uv_a, *uv_b;

                        if (!is_0_clipped){ 
                            unclipped_point = &cur_r_face->position_0;
                            point_a = &cur_r_face->position_1;
                            point_b = &cur_r_face->position_2;
                            
                            unclipped_normal = &cur_r_face->normal_0;
                            normal_a = &cur_r_face->normal_1;
                            normal_b = &cur_r_face->normal_2;
                        
                            unclipped_uv = &cur_r_face->uv_0;
                            uv_a = &cur_r_face->uv_1;
                            uv_b = &cur_r_face->uv_2;
                        }
                        else if (!is_1_clipped){ 
                            unclipped_point = &cur_r_face->position_1;
                            point_a = &cur_r_face->position_2;
                            point_b = &cur_r_face->position_0;
                            
                            unclipped_normal = &cur_r_face->normal_1;
                            normal_a = &cur_r_face->normal_2;
                            normal_b = &cur_r_face->normal_0;
                        
                            unclipped_uv = &cur_r_face->uv_1;
                            uv_a = &cur_r_face->uv_2;
                            uv_b = &cur_r_face->uv_0;
                        }
                        else if (!is_2_clipped){ 
                            unclipped_point = &cur_r_face->position_2;
                            point_a = &cur_r_face->position_0;
                            point_b = &cur_r_face->position_1;
                            
                            unclipped_normal = &cur_r_face->normal_2;
                            normal_a = &cur_r_face->normal_0;
                            normal_b = &cur_r_face->normal_1;
                        
                            unclipped_uv = &cur_r_face->uv_2;
                            uv_a = &cur_r_face->uv_0;
                            uv_b = &cur_r_face->uv_1;
                        }
                    
                        double fraction_a_to_unclip = (scene->min_clip - unclipped_point->z) / (point_a->z - unclipped_point->z);                          
                        double fraction_b_to_unclip = (scene->min_clip - unclipped_point->z) / (point_b->z - unclipped_point->z);  

                        vector_3f_lerp(*unclipped_point, *point_a, point_a, fraction_a_to_unclip);
                        vector_3f_lerp(*unclipped_point, *point_b, point_b, fraction_b_to_unclip);

                        vector_3f_lerp(*unclipped_normal, *normal_a, normal_a, fraction_a_to_unclip);
                        vector_3f_lerp(*unclipped_normal, *normal_b, normal_b, fraction_b_to_unclip);

                        vector_2f_lerp(*unclipped_uv, *uv_a, uv_a, fraction_a_to_unclip);
                        vector_2f_lerp(*unclipped_uv, *uv_b, uv_b, fraction_b_to_unclip);

                        break;}

                    case 1: // split polygon
                        RI_vector_3f clipped_point, point_a, point_b;
                        RI_vector_3f clipped_normal, normal_a, normal_b;
                        RI_vector_2f clipped_uv, uv_a, uv_b;

                        if (is_0_clipped){ 
                            clipped_point = cur_r_face->position_0;
                            point_a = cur_r_face->position_1;
                            point_b = cur_r_face->position_2;
                            
                            clipped_normal = cur_r_face->normal_0;
                            normal_a = cur_r_face->normal_1;
                            normal_b = cur_r_face->normal_2;
                        
                            clipped_uv = cur_r_face->uv_0;
                            uv_a = cur_r_face->uv_1;
                            uv_b = cur_r_face->uv_2;
                        }
                        else if (is_1_clipped){ 
                            clipped_point = cur_r_face->position_1;
                            point_a = cur_r_face->position_2;
                            point_b = cur_r_face->position_0;
                            
                            clipped_normal = cur_r_face->normal_1;
                            normal_a = cur_r_face->normal_2;
                            normal_b = cur_r_face->normal_0;
                        
                            clipped_uv = cur_r_face->uv_1;
                            uv_a = cur_r_face->uv_2;
                            uv_b = cur_r_face->uv_0;
                        }
                        else if (is_2_clipped){ 
                            clipped_point = cur_r_face->position_2;
                            point_a = cur_r_face->position_0;
                            point_b = cur_r_face->position_1;
                            
                            clipped_normal = cur_r_face->normal_2;
                            normal_a = cur_r_face->normal_0;
                            normal_b = cur_r_face->normal_1;
                        
                            clipped_uv = cur_r_face->uv_2;
                            uv_a = cur_r_face->uv_0;
                            uv_b = cur_r_face->uv_1;
                        }

                        double fraction_a_to_clip = (scene->min_clip - clipped_point.z) / (point_a.z - clipped_point.z);                        
                        double fraction_b_to_clip = (scene->min_clip - clipped_point.z) / (point_b.z - clipped_point.z);                        

                        RI_vector_3f new_point_a, new_point_b;  // the new points that move along the polygon's edge to match the z value of min_clip.
                        RI_vector_3f new_normal_a, new_normal_b;  // they come from the clipped point which was originally only 1
                        RI_vector_2f new_uv_a, new_uv_b;
                        
                        vector_3f_lerp(clipped_point, point_a, &new_point_a, fraction_a_to_clip);
                        vector_3f_lerp(clipped_point, point_b, &new_point_b, fraction_b_to_clip);
                        
                        vector_3f_lerp(clipped_normal, normal_a, &new_normal_a, fraction_a_to_clip);
                        vector_3f_lerp(clipped_normal, normal_b, &new_normal_b, fraction_b_to_clip);
                        
                        vector_2f_lerp(clipped_uv, uv_a, &new_uv_a, fraction_a_to_clip);
                        vector_2f_lerp(clipped_uv, uv_b, &new_uv_b, fraction_b_to_clip);

                        // okay, now we have a quad (in clockwise order, point a, point b, new point b, new point a)
                        // quads are easy to turn into tris >w<

                        RI_renderable_face *cur_r_split_face = &scene->faces_to_render[scene->face_count + current_split_renderable_face_index];

                        cur_r_split_face->parent_actor = current_actor;

                        cur_r_split_face->should_render = 1;

                        cur_r_split_face->material_reference = cur_r_face->material_reference;

                        cur_r_face->position_0 = point_a;
                        cur_r_face->position_1 = point_b;
                        cur_r_face->position_2 = new_point_a;

                        cur_r_face->normal_0 = normal_a;
                        cur_r_face->normal_1 = normal_b;
                        cur_r_face->normal_2 = new_normal_a;

                        cur_r_face->uv_0 = uv_a;
                        cur_r_face->uv_1 = uv_b;
                        cur_r_face->uv_2 = new_uv_a;

                        cur_r_split_face->position_0 = point_b;
                        cur_r_split_face->position_1 = new_point_b;
                        cur_r_split_face->position_2 = new_point_a;

                        cur_r_split_face->normal_0 = normal_b;
                        cur_r_split_face->normal_1 = new_normal_b;
                        cur_r_split_face->normal_2 = new_normal_a;

                        cur_r_split_face->uv_0 = uv_b;
                        cur_r_split_face->uv_1 = new_uv_b;
                        cur_r_split_face->uv_2 = new_uv_a;

                        cur_r_split_face->position_0.x = cur_r_split_face->position_0.x / cur_r_split_face->position_0.z * horizontal_fov_factor;
                        cur_r_split_face->position_0.y = cur_r_split_face->position_0.y / cur_r_split_face->position_0.z * vertical_fov_factor;
                        
                        cur_r_split_face->position_1.x = cur_r_split_face->position_1.x / cur_r_split_face->position_1.z * horizontal_fov_factor;
                        cur_r_split_face->position_1.y = cur_r_split_face->position_1.y / cur_r_split_face->position_1.z * vertical_fov_factor;

                        cur_r_split_face->position_2.x = cur_r_split_face->position_2.x / cur_r_split_face->position_2.z * horizontal_fov_factor;
                        cur_r_split_face->position_2.y = cur_r_split_face->position_2.y / cur_r_split_face->position_2.z * vertical_fov_factor;

                        cur_r_split_face->min_screen_x = cur_r_split_face->position_0.x; 
                        if (cur_r_split_face->position_1.x < cur_r_split_face->min_screen_x) cur_r_split_face->min_screen_x = cur_r_split_face->position_1.x;
                        if (cur_r_split_face->position_2.x < cur_r_split_face->min_screen_x) cur_r_split_face->min_screen_x = cur_r_split_face->position_2.x;
                        cur_r_split_face->min_screen_x = fmax(cur_r_split_face->min_screen_x, -target_texture->resolution.x / 2); 

                        cur_r_split_face->max_screen_x = cur_r_split_face->position_0.x; 
                        if (cur_r_split_face->position_1.x > cur_r_split_face->max_screen_x) cur_r_split_face->max_screen_x = cur_r_split_face->position_1.x;
                        if (cur_r_split_face->position_2.x > cur_r_split_face->max_screen_x) cur_r_split_face->max_screen_x = cur_r_split_face->position_2.x;
                        cur_r_split_face->max_screen_x = fmin(cur_r_split_face->max_screen_x, target_texture->resolution.x / 2); 

                        cur_r_split_face->min_screen_y = cur_r_split_face->position_0.y; 
                        if (cur_r_split_face->position_1.y < cur_r_split_face->min_screen_y) cur_r_split_face->min_screen_y = cur_r_split_face->position_1.y;
                        if (cur_r_split_face->position_2.y < cur_r_split_face->min_screen_y) cur_r_split_face->min_screen_y = cur_r_split_face->position_2.y;
                        cur_r_split_face->min_screen_y = fmax(cur_r_split_face->min_screen_y, -target_texture->resolution.y / 2); 

                        cur_r_split_face->max_screen_y = cur_r_split_face->position_0.y; 
                        if (cur_r_split_face->position_1.y > cur_r_split_face->max_screen_y) cur_r_split_face->max_screen_y = cur_r_split_face->position_1.y;
                        if (cur_r_split_face->position_2.y > cur_r_split_face->max_screen_y) cur_r_split_face->max_screen_y = cur_r_split_face->position_2.y;
                        cur_r_split_face->max_screen_y = fmin(cur_r_split_face->max_screen_y, target_texture->resolution.y / 2); 

                        ++current_split_renderable_face_index;
                        
                        break;
                    
                    case 0: // no issues, ignore
                        break;
                }

                cur_r_face->position_0.x = cur_r_face->position_0.x / cur_r_face->position_0.z * horizontal_fov_factor;
                cur_r_face->position_0.y = cur_r_face->position_0.y / cur_r_face->position_0.z * vertical_fov_factor;
                
                cur_r_face->position_1.x = cur_r_face->position_1.x / cur_r_face->position_1.z * horizontal_fov_factor;
                cur_r_face->position_1.y = cur_r_face->position_1.y / cur_r_face->position_1.z * vertical_fov_factor;

                cur_r_face->position_2.x = cur_r_face->position_2.x / cur_r_face->position_2.z * horizontal_fov_factor;
                cur_r_face->position_2.y = cur_r_face->position_2.y / cur_r_face->position_2.z * vertical_fov_factor;

                cur_r_face->min_screen_x = pos_0->x; 
                if (pos_1->x < cur_r_face->min_screen_x) cur_r_face->min_screen_x = pos_1->x;
                if (pos_2->x < cur_r_face->min_screen_x) cur_r_face->min_screen_x = pos_2->x;
                cur_r_face->min_screen_x = fmax(cur_r_face->min_screen_x, -target_texture->resolution.x / 2); 

                cur_r_face->max_screen_x = pos_0->x; 
                if (pos_1->x > cur_r_face->max_screen_x) cur_r_face->max_screen_x = pos_1->x;
                if (pos_2->x > cur_r_face->max_screen_x) cur_r_face->max_screen_x = pos_2->x;
                cur_r_face->max_screen_x = fmin(cur_r_face->max_screen_x, target_texture->resolution.x / 2); 

                cur_r_face->min_screen_y = pos_0->y; 
                if (pos_1->y < cur_r_face->min_screen_y) cur_r_face->min_screen_y = pos_1->y;
                if (pos_2->y < cur_r_face->min_screen_y) cur_r_face->min_screen_y = pos_2->y;
                cur_r_face->min_screen_y = fmax(cur_r_face->min_screen_y, -target_texture->resolution.y / 2); 

                cur_r_face->max_screen_y = pos_0->y; 
                if (pos_1->y > cur_r_face->max_screen_y) cur_r_face->max_screen_y = pos_1->y;
                if (pos_2->y > cur_r_face->max_screen_y) cur_r_face->max_screen_y = pos_2->y;
                cur_r_face->max_screen_y = fmin(cur_r_face->max_screen_y, target_texture->resolution.y / 2); 

                ++current_renderable_face_index;
            }
        }

        if (ri.z_buffer_resolution.x * ri.z_buffer_resolution.y < target_texture->resolution.x * target_texture->resolution.y){
            ri.z_buffer = RI_realloc(ri.z_buffer, sizeof(double) * target_texture->resolution.x * target_texture->resolution.y);
        }

        for (int pixel_index = 0; pixel_index < target_texture->resolution.x * target_texture->resolution.y; ++pixel_index){
            target_texture->image_buffer[pixel_index] = 0xFF333333;
            ri.z_buffer[pixel_index] = 999999999;
        }

        for (int face_index = 0; face_index < current_renderable_face_index * 2; ++face_index){
            RI_renderable_face *current_face = &scene->faces_to_render[face_index];
            
            if (!current_face->should_render) continue;

            RI_material *mat = current_face->material_reference;
        
            RI_vector_2f *uv_0 = &current_face->uv_0;;
            RI_vector_2f *uv_1 = &current_face->uv_1;;
            RI_vector_2f *uv_2 = &current_face->uv_2;;
        
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
        
            RI_vector_3f *pos_0 = &current_face->position_0;
            RI_vector_3f *pos_1 = &current_face->position_1;
            RI_vector_3f *pos_2 = &current_face->position_2;

            for (int pixel_y_index = current_face->min_screen_y; pixel_y_index < current_face->max_screen_y; ++pixel_y_index){    
                for (int pixel_x_index = current_face->min_screen_x; pixel_x_index < current_face->max_screen_x; ++pixel_x_index){                   
                    int x = pixel_x_index + target_texture->resolution.x / 2;
                    int y = pixel_y_index + target_texture->resolution.y / 2;

                    if (x < 0 || x >= target_texture->resolution.x || y < 0 || y >= target_texture->resolution.y) continue;

                    double denominator, w0, w1, w2;
                
                    denominator = (pos_1->y - pos_2->y) * (pos_0->x - pos_2->x) + (pos_2->x - pos_1->x) * (pos_0->y - pos_2->y);
                    w0 = ((pos_1->y - pos_2->y) * (pixel_x_index - pos_2->x) + (pos_2->x - pos_1->x) * (pixel_y_index - pos_2->y)) / denominator;
                    w1 = ((pos_2->y - pos_0->y) * (pixel_x_index - pos_0->x) + (pos_0->x - pos_2->x) * (pixel_y_index - pos_0->y)) / denominator; 
                    w2 = 1.0 - w0 - w1; 
                
                    if (!(mat->flags & RI_MATERIAL_DOUBLE_SIDED) && denominator > 0){
                        continue;
                    }

                    double w_over_z = (w0 / pos_0->z + w1 / pos_1->z + w2 / pos_2->z); 
                    double interpolated_z = 1.0 / w_over_z;
                
                    if (!(w0 >= 0 && w1 >= 0 && w2 >= 0) || (mat->flags & RI_MATERIAL_WIREFRAME && (w0 >= mat->wireframe_width && w1 >= mat->wireframe_width && w2 >= mat->wireframe_width))){
                        continue;
                    }
                    
                    if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_TEST) && interpolated_z >= ri.z_buffer[y * target_texture->resolution.x + x]){
                        continue;
                    }   
                
                    if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_WRITE)){
                        ri.z_buffer[y * target_texture->resolution.x + x] = interpolated_z;
                    }
                    
                    uint32_t pixel_color = 0xFF000000;
                    
                    if (mat->flags & RI_MATERIAL_HAS_TEXTURE && uv_0 && uv_1 && uv_2){                
                        double ux = (w0 * (uv_0->x / pos_0->z) + w1 * (uv_1->x / pos_1->z) + w2 * (uv_2->x / pos_2->z)) / w_over_z;
                        double uy = (w0 * (uv_0->y / pos_0->z) + w1 * (uv_1->y / pos_1->z) + w2 * (uv_2->y / pos_2->z)) / w_over_z;                
                    
                        if (mat->flags & RI_MATERIAL_USE_UV_LOOP_MULTIPLIER){
                            ux *= mat->uv_loop_multiplier.x;
                            uy *= mat->uv_loop_multiplier.y;
                        }

                        if (mat->flags & RI_MATERIAL_USE_UV_RENDER_RESOLUTION){
                            ux *= (current_face->parent_actor->transform.scale.x / mat->texture_render_size.x);
                            uy *= (current_face->parent_actor->transform.scale.y / mat->texture_render_size.y);
                        }

                        ux = mod(ux, 1.0);
                        uy = mod(-uy, 1.0);

                        RI_vector_2 texel_position = {mat->texture_reference->resolution.x * ux, mat->texture_reference->resolution.y * uy};
                        
                        if (texel_position.y * mat->texture_reference->resolution.x + texel_position.x < 0 || texel_position.y * mat->texture_reference->resolution.x + texel_position.x >= mat->texture_reference->resolution.x * mat->texture_reference->resolution.y) 
                            pixel_color = 0xFFFF00FF;
                        else 
                            pixel_color = mat->texture_reference->image_buffer[texel_position.y * mat->texture_reference->resolution.x + texel_position.x];
                    }
                    else { // must be only an albedo
                        if (mat->albedo) pixel_color = mat->albedo;
                        else pixel_color = 0xFFFF77FF;
                    }

                    // tri culling debug
                    // if (face_index >= current_renderable_face_index) pixel_color = 0xFF7777FF;
                    // if (face_index < scene->face_count) pixel_color = 0xFF77FF77;
                
                    // flip the texture
                    // x = target_texture->resolution.x - 1 - x;
                    // y = target_texture->resolution.y - 1 - y;
                
                    if (x >= 0 && y >= 0 && x < target_texture->resolution.x && y < target_texture->resolution.y){
                        target_texture->image_buffer[y * target_texture->resolution.x + x] = pixel_color;
                    }
                }
            }
        }

    }
    else{
        RI_stop(0);
    }

    return 0;
}

void RI_tick(){
    SDL_UpdateTexture(ri.texture, NULL, ri.frame_buffer->image_buffer, ri.window_width * sizeof(uint32_t));

    SDL_RenderClear(ri.renderer);
    SDL_RenderCopyEx(ri.renderer, ri.texture, NULL, NULL, 0, NULL, SDL_FLIP_VERTICAL);

    SDL_RenderPresent(ri.renderer);

    // handle SDL events
    while (SDL_PollEvent(&ri.event)){
        switch (ri.event.type){
            case SDL_QUIT:
                ri.running = 0;
        }
    }

    ++ri.frame;
}

int opencl_init(){
    return 0;
}

int sdl_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.window_width = RI_window_width;
    ri.window_height = RI_window_height;
    ri.window_title = RI_window_title;

    ri.frame_buffer = RI_malloc(sizeof(RI_texture));

    ri.frame_buffer->image_buffer = RI_malloc(sizeof(uint32_t) * ri.window_width * ri.window_height);
    ri.frame_buffer->resolution = (RI_vector_2){ri.window_width, ri.window_height};
    
    ri.z_buffer = RI_malloc(sizeof(double) * ri.window_width * ri.window_height);
    ri.z_buffer_resolution = (RI_vector_2){ri.window_width, ri.window_height};

    SDL_Init(SDL_INIT_VIDEO);

    ri.window = SDL_CreateWindow(RI_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ri.window_width, ri.window_height, SDL_WINDOW_OPENGL);

    ri.renderer = SDL_CreateRenderer(ri.window, -1, SDL_RENDERER_ACCELERATED);

    ri.texture = SDL_CreateTexture(ri.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ri.window_width, ri.window_height);

    return 0;
}

int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    ri.running = 1;

    ri.prefix = "[RasterIver] ";

    if (ri.debug_memory){
        ri.current_allocation_index = 0;
        ri.allocation_search_limit = 100;
        ri.allocation_table_length = 100;
        
        size_t __size = sizeof(RI_memory_allocation) * ri.allocation_table_length;

        ri.allocation_table = malloc(__size);
    
        debug("[Memory Manager] Allocated (malloc) %zu bytes", __size);

        ri.allocation_table[ri.current_allocation_index].allocated = 1;
        ri.allocation_table[ri.current_allocation_index].freed = 0;
        ri.allocation_table[ri.current_allocation_index].reallocated_alloc = 0;
        ri.allocation_table[ri.current_allocation_index].reallocated_free = 0;
        ri.allocation_table[ri.current_allocation_index].pointer = ri.allocation_table;        
        ri.allocation_table[ri.current_allocation_index].size = __size;

        ri.current_allocation_index++;
    }

    opencl_init();

    sdl_init(RI_window_width, RI_window_height, RI_window_title);

    ri.loaded_mesh_count = 0;
    ri.loaded_texture_count = 0;
    ri.actor_count = 0;

    char **error_cube_file = RI_malloc(sizeof(char *));
    error_cube_file[0] = "objects/unit_cube.obj";

    RI_mesh* error_mesh = RI_request_meshes(1, error_cube_file, 1);

    ri.error_mesh = *error_mesh;

    RI_free(error_mesh);
    RI_free(error_cube_file);

    ri.error_texture.image_buffer = RI_malloc(sizeof(uint32_t));

    ri.error_texture.image_buffer[0] = 0xFFFF00FF;
    ri.error_texture.resolution = (RI_vector_2){1, 1};

    ri.error_material.texture_reference = &ri.error_texture;
    ri.error_material.albedo = 0xFF5522CC;
    ri.error_material.flags = RI_MATERIAL_UNLIT | RI_MATERIAL_DONT_DEPTH_TEST | RI_MATERIAL_DONT_RECEIVE_SHADOW | RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;

    return 0;
}

int RI_stop(int result){
    debug("[Notice] Stopping...");
    
    for (int scene_index = 0; scene_index < ri.scene_count; ++scene_index){
        RI_free(ri.scenes[scene_index].faces_to_render);
        RI_free(ri.scenes[scene_index].actors);
    }

    for (int mesh_index = 0; mesh_index < ri.loaded_mesh_count; ++mesh_index){
        RI_free(ri.loaded_meshes[mesh_index].faces);
        RI_free(ri.loaded_meshes[mesh_index].vertex_positions);
        RI_free(ri.loaded_meshes[mesh_index].normals);
        RI_free(ri.loaded_meshes[mesh_index].uvs);
    }

    for (int texture_index = 0; texture_index < ri.loaded_texture_count; ++texture_index){
        RI_free(ri.loaded_textures[texture_index].image_buffer);
    }

    RI_free(ri.loaded_meshes);
    RI_free(ri.loaded_textures);
    RI_free(ri.materials);
    RI_free(ri.actors);
    RI_free(ri.scenes);

    RI_free(ri.error_texture.image_buffer);
    RI_free(ri.frame_buffer->image_buffer);
    RI_free(ri.frame_buffer);
    RI_free(ri.z_buffer);

    RI_free(ri.error_mesh.faces);
    RI_free(ri.error_mesh.vertex_positions);
    RI_free(ri.error_mesh.normals);
    RI_free(ri.error_mesh.uvs);

    if (ri.debug_memory){
        size_t total_allocated = 0;
        size_t allocated = 0;
        size_t alloc_realloc = 0;
        size_t total_freed = 0;
        size_t freed = 0;
        size_t reallocated = 0;

        for (int i = 1; i < ri.allocation_table_length; ++i) {
            if (ri.allocation_table[i].allocated != 1) continue;
            else if (ri.allocation_table[i].freed)
                freed += ri.allocation_table[i].size;
                else debug("[Memory Manager] Memory allocated at line %d wasn't freed (%zu bytes)", ri.allocation_table[i].line, ri.allocation_table[i].size);
            
            if (!ri.allocation_table[i].reallocated_free && !ri.allocation_table[i].reallocated_alloc)
                allocated += ri.allocation_table[i].size;
            else if (ri.allocation_table[i].reallocated_alloc)
                alloc_realloc += ri.allocation_table[i].size;
            else if (ri.allocation_table[i].reallocated_free)
                reallocated += ri.allocation_table[i].size;
        }

        total_allocated = allocated + alloc_realloc;
        total_freed = freed + reallocated;

        debug("[Memory Manager] [Total Bytes Allocated] M(c)alloc & Realloc(): %zu -- M(c)alloc(): %zu -- Realloc(): %zu", total_allocated, allocated, alloc_realloc);
        debug("[Memory Manager] [Total Bytes Freed] Free() & Realloc(): %zu -- Free(): %zu -- Realloc(): %zu", total_freed, freed, reallocated);

        if (total_allocated != total_freed){
            debug("[Memory Manager] %zu bytes not freed", total_allocated - total_freed);
        }
    
        debug("[Memory Manager] Freeing allocation table...");
    
        RI_free(ri.allocation_table);
    }

    exit(result);

    return 0;
}