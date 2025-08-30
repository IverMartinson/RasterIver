#include <stdio.h>
#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"
#include "stdint.h"
#include <math.h>
#include <signal.h>

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

RI_texture* RI_request_empty_texture(RI_vector_2 resolution){
    int previous_loaded_texture_count = ri.loaded_texture_count;

    ri.loaded_texture_count++;

    ri.loaded_textures = RI_realloc(ri.loaded_textures, sizeof(RI_texture) * ri.loaded_texture_count);

    ri.loaded_textures[previous_loaded_texture_count].image_buffer = RI_malloc(sizeof(uint32_t) * resolution.x * resolution.y);
    ri.loaded_textures[previous_loaded_texture_count].resolution = resolution;

    return &ri.loaded_textures[previous_loaded_texture_count];
}

void RI_wipe_texture(RI_texture *target_texture, uint32_t color){
    for (int i = 0; i < target_texture->resolution.x * target_texture->resolution.y; ++i){
        target_texture->image_buffer[i] = color;
    }
}

void RI_draw_line(RI_texture *target_texture, RI_vector_2 point_a, RI_vector_2 point_b, uint32_t color){
    int num_pixels = distance_2(point_a, point_b);
    
    for (int i = 0; i < num_pixels; ++i){
        RI_vector_2 point_2_draw; vector_2_lerp(point_a, point_b, &point_2_draw, (double)i / (double)num_pixels);

        if (point_2_draw.x < 0 || point_2_draw.x >= target_texture->resolution.x || point_2_draw.y < 0 || point_2_draw.y >= target_texture->resolution.y) continue;

        target_texture->image_buffer[point_2_draw.y * target_texture->resolution.x + point_2_draw.x] = color;
    }
}

int ccw(RI_vector_2f a, RI_vector_2f b, RI_vector_2f c) {
    return (c.y - a.y) * (b.x - a.x) > (b.y - a.y) * (c.x - a.x);
}

int intersects_segments(RI_vector_2f a, RI_vector_2f b, RI_vector_2f c, RI_vector_2f d) {
    return (ccw(a, c, d) != ccw(b, c, d)) && (ccw(a, b, c) != ccw(a, b, d));
}

int intersects(RI_vector_2f a, RI_vector_2f b, RI_vector_2f c){
    RI_vector_2f d = {10000, c.y};
    return intersects_segments(a, b, c, d);
}

void render_glyph(RI_texture *target_texture, RI_vector_2f position, double size, uint32_t color, int bezier_resolution, double units_per_em, SP_glyph *glyph){
    // this function doesn't use the RI allocation wrappers because 
    // everything in it is freed by the end, 
    // so it just wastes memory trying to track it
    
    // estimate
    int new_point_count = 0;
    int allocated_new_points = glyph->number_of_points * 3;
    RI_vector_2f *new_points = malloc(sizeof(RI_vector_2f) * allocated_new_points);
    
    int *contour_ends = malloc(sizeof(int) * glyph->number_of_contours);
    
    for (int contour = 0; contour < glyph->number_of_contours; ++contour){
        // if we are at contour 0, point_start is 0.
        // if contour is > 0 but != 0, point_start is equal to the previous index
        int point_start = contour > 0 ? (glyph->contour_end_indicies[contour - 1] + 1) : 0;

        int point_offset = point_start;

        // find first on-curve point because the first point isn't always on the curves
        // while (!(glyph->flags[point_offset] & 1)){
            // point_offset++;
        // }

        for (int point = 0; point <= glyph->contour_end_indicies[contour] + 1 - point_start; ++point){
            if (new_point_count + 1 >= allocated_new_points){
                allocated_new_points += 20;

                new_points = realloc(new_points, sizeof(RI_vector_2f) * allocated_new_points);
            }
            
            int cur = (point % (glyph->contour_end_indicies[contour] + 1 - point_start) + point_offset);
            int next = ((point + 1) % (glyph->contour_end_indicies[contour] + 1 - point_start) + point_offset);

            new_points[new_point_count].x = (double)glyph->x_coords[cur] / units_per_em * size + position.x;
            new_points[new_point_count].y = (double)glyph->y_coords[cur] / units_per_em * size + position.y;

            new_point_count++;

            // if current and next glyph are both on or off the curve, add a point between them-
            if ((glyph->flags[cur] & 1) == (glyph->flags[next] & 1)){
                vector_2f_lerp((RI_vector_2f){(double)glyph->x_coords[cur] / units_per_em * size + position.x, (double)glyph->y_coords[cur] / units_per_em * size + position.y}, (RI_vector_2f){(double)glyph->x_coords[next] / units_per_em * size + position.x, (double)glyph->y_coords[next] / units_per_em * size + position.y}, &new_points[new_point_count], 0.5);

                new_point_count++;
            }
        }

        contour_ends[contour] = new_point_count;
    }
    
    allocated_new_points = new_point_count;

    // maybe make multiple arrays that are lists of lines in increments of y values so that we dont have to check every line even if its above the pixel (the ray only goes to the right)

    RI_vector_2f *lines = malloc(sizeof(RI_vector_2f) * (new_point_count / 2) * (bezier_resolution <= 1 ? 2 : bezier_resolution) + 30);

    for (int contour = 0; contour < glyph->number_of_contours; ++contour){
        int p_start = (contour > 0 ? contour_ends[contour - 1] : 0);
        int p_end = contour_ends[contour];

        for (int point = p_start; point < p_end - 2; point += 2){
            RI_vector_2f point_a = new_points[point];
            RI_vector_2f point_b = new_points[(point + 1)];
            RI_vector_2f point_c = new_points[(point + 2)];
            
            RI_vector_2f prev_point = new_points[point];

            for (int i = 0; i < bezier_resolution; ++i){
                double w = (double)(i + 1) / (double)bezier_resolution;
                
                RI_vector_2f bez_point; 
                
                if (bezier_resolution <= 1) bez_point = point_c;
                else vector_2f_bezier_interpolate(point_a, point_b, point_c, &bez_point, w);

                lines[point + i] = prev_point;
                lines[point + i + 1] = bez_point;

                prev_point = bez_point;
            }
        }
    }

    for (int y = fmax((int)((double)glyph->y_min / (double)units_per_em * size) + position.y, 0.0); y < fmin((int)((double)glyph->y_max / (double)units_per_em * size) + position.y, target_texture->resolution.y); ++y){
        for (int x = fmax((int)((double)glyph->x_min / (double)units_per_em * size) + position.x, 0.0); x < fmin((int)((double)glyph->x_max / (double)units_per_em * size) + position.x, target_texture->resolution.x); ++x){
            int intersections = 0;
            
            for (int contour = 0; contour < glyph->number_of_contours; ++contour){
                int p_start = (contour > 0 ? contour_ends[contour - 1] : 0);
                int p_end = contour_ends[contour];
        
                for (int point = p_start; point < p_end - 2; point += 2){ 
                    for (int i = 0; i < bezier_resolution; ++i){

                        if(intersects(lines[point + i], lines[point + i + 1], (RI_vector_2f){x, y})) intersections++; 
                    }  
                }
            }

            if (intersections % 2 != 0) target_texture->image_buffer[y * target_texture->resolution.x + x] = color;
        }
    }

    free(lines);
    free(new_points);
    free(contour_ends);
}

void RI_render_text(SP_font *font, RI_texture *target_texture, RI_vector_2f position, uint32_t color, int bezier_resolution, double size, char *text){
    int character_count = strlen(text);
    
    int current_advance_width = 0;
    RI_vector_2f glyph_position = position;

    for (int character_i = 0; character_i < character_count; ++character_i){
        int glyph = font->unicode_to_glyph_indicies[text[character_i]];
        
        if (glyph >= font->number_of_glyphs) glyph = 0;
        
        SP_glyph *current_glyph = &font->glyphs[glyph];
        
        glyph_position.x = position.x;// + ((double)font->h_metrics[glyph].left_side_bearing / font->units_per_em * size);
        
        if (current_glyph->number_of_contours > 0){
            render_glyph(target_texture, glyph_position, size, color, bezier_resolution, font->units_per_em, current_glyph);
        }
        else if (current_glyph->number_of_contours == 0){
            // do nothing, glyph is empty
        }
        else {
            for (int k = 0; k < current_glyph->number_of_components; k++){
                int c_glyph = current_glyph->components[k].glyph_index;
                
                double offset_x = (double)current_glyph->components[k].arg1;
                double offset_y = (double)current_glyph->components[k].arg2;

                glyph_position.x += offset_x / font->units_per_em * size;
                glyph_position.y += offset_y / font->units_per_em * size;
                
                render_glyph(target_texture, glyph_position, size, color, bezier_resolution, font->units_per_em, &font->glyphs[c_glyph]);
                
                glyph_position.x -= offset_x / font->units_per_em * size;
                glyph_position.y -= offset_y / font->units_per_em * size;
            }
        }

        // find kerning value
        double kerning_offset = 0;
        
        if (character_i + 1 < character_count && font->number_of_kerning_pairs > 0){
            uint32_t key = (glyph << 16) | (uint16_t)font->unicode_to_glyph_indicies[text[character_i + 1]];
            
            int low = 0;
            int high = font->number_of_kerning_pairs - 1;
            
            while (low <= high) {
                int mid = (low + high) / 2;
                uint32_t midKey = (font->kerning_pairs[mid].left << 16) | font->kerning_pairs[mid].right;

                if (midKey == key) {
                    kerning_offset = (double)font->kerning_pairs[mid].value; // found it
                    break;
                } else if (midKey < key) {
                    low = mid + 1;           // search right half
                } else {
                    high = mid - 1;          // search left half
                }
            }
        }

        position.x += (double)(font->h_metrics[glyph].advance_width + kerning_offset) / font->units_per_em * size;
    }
}

int RI_add_actors_to_scene(int RI_number_of_actors_to_add_to_scene, RI_actor **actors, RI_scene *scene){
    int previous_actor_count = scene->actor_count;

    scene->actor_count += RI_number_of_actors_to_add_to_scene;

    scene->actors = RI_realloc(scene->actors, sizeof(RI_actor *) * scene->actor_count);

    for (int i = 0; i < RI_number_of_actors_to_add_to_scene; ++i){
        scene->actors[i + previous_actor_count] = actors[i];
    }

    return 0;
}

RI_scene* RI_request_scene(int RI_number_of_requested_scenes){
    RI_scene *new_scene = RI_malloc(sizeof(RI_scene));

    new_scene->actor_count = 0;
    new_scene->actors = NULL;
    new_scene->faces_to_render = NULL;
    new_scene->antialiasing_subsample_resolution = 4;
    new_scene->camera_position = (RI_vector_3f){0, 0, 0};
    new_scene->camera_rotation = (RI_vector_4f){1, 0, 0, 0};
    new_scene->face_count = 0;
    new_scene->faces_to_render = NULL;
    new_scene->flags = 0;
    new_scene->FOV = 3.14159 / 2;
    new_scene->min_clip = 0.01;
    new_scene->minimum_clip_distance = 0.01;

    return new_scene;
}

RI_actor* RI_request_actor(int RI_number_of_requested_actors){
    RI_actor *new_actor = RI_malloc(sizeof(RI_scene));

    new_actor->mesh_reference = NULL;
    new_actor->material_reference = NULL;
    new_actor->transform.position = (RI_vector_3f){0, 0, 0};
    new_actor->transform.scale = (RI_vector_3f){1, 1, 1};
    new_actor->transform.rotation = (RI_vector_4f){1, 0, 0, 0};

    return new_actor;
}

RI_material* RI_request_material(){
    RI_material *new_material = RI_malloc(sizeof(RI_actor));

    *new_material = ri.error_material;
    
    return new_material;
}

RI_texture* RI_request_texture(char *filename){
    RI_texture *new_texture = RI_malloc(sizeof(RI_texture));

    unsigned char* temp_texture = stbi_load(filename, &new_texture->resolution.x, &new_texture->resolution.y, NULL, 4);
    
    if(stbi_failure_reason()){
        *new_texture = ri.error_texture;
    }
    else {
        new_texture->image_buffer = RI_malloc(sizeof(uint32_t) * new_texture->resolution.x * new_texture->resolution.y);

        for (int i = 0; i < new_texture->resolution.x * new_texture->resolution.y; ++i){
            unsigned char r = temp_texture[i * 4];
            unsigned char g = temp_texture[i * 4 + 1];
            unsigned char b = temp_texture[i * 4 + 2];
            unsigned char a = temp_texture[i * 4 + 3];
            
            new_texture->image_buffer[i] = (a << 24 | r << 16 | g << 8 | b);
        }
    }

    stbi_image_free(temp_texture);

    return new_texture;
}

RI_mesh* RI_request_mesh(char *filename){
    RI_mesh *new_mesh = RI_calloc(1, sizeof(RI_mesh));

    FILE *file = fopen(filename, "r");

    if (!file){
        debug("[Mesh Loader] Error! File \"%s\" not found", filename);
        RI_stop(1);
    }
    
    char line[512];
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') { // face
            new_mesh->face_count++;
        }
        else if (line[0] == 'v' && line[1] == ' ') { // vertex
            new_mesh->vertex_count++;
        }
        else if (line[0] == 'v' && line[1] == 'n') { // normal
            new_mesh->normal_count++;
        }
        else if (line[0] == 'v' && line[1] == 't') { // UV
            new_mesh->uv_count++;
        }
    }

    rewind(file);
        
    new_mesh->faces = RI_malloc(sizeof(RI_face) * new_mesh->face_count);
    new_mesh->vertex_positions = RI_malloc(sizeof(RI_vector_3f) * new_mesh->vertex_count);
    new_mesh->normals = RI_malloc(sizeof(RI_vector_3f) * new_mesh->normal_count);
    new_mesh->uvs = RI_malloc(sizeof(RI_vector_2f) * new_mesh->uv_count);

    int current_face_index = 0;
    int current_vertex_index = 0;
    int current_normal_index = 0;
    int current_uv_index = 0;

    int has_normals, has_uvs;
    has_normals = has_uvs = 0;

    while (fgets(line, sizeof(line), file)) {
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

            new_mesh->faces[current_face_index].position_0_index = vertex_0_index - 1;
            new_mesh->faces[current_face_index].position_1_index = vertex_1_index - 1;
            new_mesh->faces[current_face_index].position_2_index = vertex_2_index - 1;

            new_mesh->faces[current_face_index].normal_0_index = normal_0_index - 1;
            new_mesh->faces[current_face_index].normal_1_index = normal_1_index - 1;
            new_mesh->faces[current_face_index].normal_2_index = normal_2_index - 1;
            
            new_mesh->faces[current_face_index].uv_0_index = uv_0_index - 1;
            new_mesh->faces[current_face_index].uv_1_index = uv_1_index - 1;
            new_mesh->faces[current_face_index].uv_2_index = uv_2_index - 1;

            new_mesh->faces[current_face_index].should_render = 1;

            ++current_face_index;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);

            new_mesh->vertex_positions[current_vertex_index].x = x;
            new_mesh->vertex_positions[current_vertex_index].y = y;
            new_mesh->vertex_positions[current_vertex_index].z = z;

            ++current_vertex_index;
        } 
        else if (line[0] == 'v' && line[1] == 'n') {
            double x, y, z;
            
            sscanf(line, "vn %lf %lf %lf", &x, &y, &z);

            new_mesh->normals[current_normal_index].x = x;
            new_mesh->normals[current_normal_index].y = y;
            new_mesh->normals[current_normal_index].z = z;

            ++current_normal_index;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            double x, y, z;

            sscanf(line, "vt %lf %lf %lf", &x, &y, &z);

            new_mesh->uvs[current_uv_index].x = x;
            new_mesh->uvs[current_uv_index].y = y;
            // UVS are almost always 2D so we don't need Z (the type itself is a vector 2f, not 3f) 

            ++current_uv_index;
        } 
    }

    char* loading_mesh_notice_string;

    if (has_normals && !has_uvs) loading_mesh_notice_string = "normals";
    else if (!has_normals && has_uvs) loading_mesh_notice_string = "UVs";
    else if (!has_normals && !has_uvs) loading_mesh_notice_string = "normals and UVs";
    
    if (!has_normals || !has_uvs) debug("[Mesh Loader] Notice! Mesh \"%s\" is missing %s", filename, loading_mesh_notice_string);
    
    new_mesh->has_normals = has_normals;
    new_mesh->has_uvs = has_uvs;

    debug("[Mesh Loader] Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", filename, current_face_index, current_vertex_index, current_normal_index, current_uv_index); 

    fclose(file);

    return new_mesh;
}

double mod(double a, double b){
    if(b < 0.0)
        return -mod(-a, -b);   
    double ret = fmod(a, b);
    if(ret < 0.0)
        ret+=b;
    return ret;
}

uint32_t multiply_rgb(uint32_t color, double factor) {
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b =  color        & 0xFF;

    r = (uint8_t)fminf(fmaxf(r * factor, 0.0f), 255.0f);
    g = (uint8_t)fminf(fmaxf(g * factor, 0.0f), 255.0f);
    b = (uint8_t)fminf(fmaxf(b * factor, 0.0f), 255.0f);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

int RI_render(RI_scene *scene, RI_texture *target_texture, int clear_texture){
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
                cur_r_face->shrunk = 0;
                cur_r_face->split = 0;

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

                // camera position & rotation
                vector_3f_element_wise_subtract(&cur_r_face->position_0, scene->camera_position);
                vector_3f_element_wise_subtract(&cur_r_face->position_1, scene->camera_position);
                vector_3f_element_wise_subtract(&cur_r_face->position_2, scene->camera_position);

                quaternion_rotate(&cur_r_face->position_0, scene->camera_rotation);
                quaternion_rotate(&cur_r_face->position_1, scene->camera_rotation);
                quaternion_rotate(&cur_r_face->position_2, scene->camera_rotation);        

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

                        cur_r_face->shrunk = 1;     

                        break;}

                    case 1: // split polygon
                        RI_vector_3f clipped_point, point_a, point_b;
                        RI_vector_3f clipped_normal, normal_a, normal_b;
                        RI_vector_2f clipped_uv, uv_a, uv_b;

                cur_r_face->split = 1;

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

                current_actor->material_reference->vertex_shader(&cur_r_face->position_0, &cur_r_face->position_1, &cur_r_face->position_2, horizontal_fov_factor, vertical_fov_factor);

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
                if (clear_texture)
                    target_texture->image_buffer[pixel_index] = 0xFF333333;
                ri.z_buffer[pixel_index] = 999999999;
           }


        for (int face_index = 0; face_index < current_renderable_face_index * 2; ++face_index){
            RI_renderable_face *current_face = &scene->faces_to_render[face_index];
            
            if (!current_face->should_render) continue;

            RI_material *mat = current_face->material_reference;
        
            RI_vector_2f *uv_0 = &current_face->uv_0;
            RI_vector_2f *uv_1 = &current_face->uv_1;
            RI_vector_2f *uv_2 = &current_face->uv_2;
        
            RI_vector_3f *normal_0 = &current_face->normal_0;
            RI_vector_3f *normal_1 = &current_face->normal_1;
            RI_vector_3f *normal_2 = &current_face->normal_2;

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
                
                    if (!(mat->flags & RI_MATERIAL_DOUBLE_SIDED || scene->flags & RI_SCENE_DOUBLE_SIDED) && denominator > 0){
                        continue;
                    }

                    double w_over_z = (w0 / pos_0->z + w1 / pos_1->z + w2 / pos_2->z); 
                    double interpolated_z = 1.0 / w_over_z;

                    if (scene->flags & RI_SCENE_DEBUG_AABB) {
                        target_texture->image_buffer[y * target_texture->resolution.x + x] += 0x0F0F0707;
                        continue;
                    }

                    if (!(w0 >= 0 && w1 >= 0 && w2 >= 0) || ((mat->flags & RI_MATERIAL_WIREFRAME || scene->flags & RI_SCENE_WIREFRAME) && (w0 >= mat->wireframe_width && w1 >= mat->wireframe_width && w2 >= mat->wireframe_width))){    
                        continue;
                    }
                    
                    if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_TEST) && interpolated_z >= ri.z_buffer[y * target_texture->resolution.x + x]){                        
                        continue;
                    }

                    if (scene->flags & RI_SCENE_DEBUG_OVERDRAW) {
                        target_texture->image_buffer[y * target_texture->resolution.x + x] += 0x0F070F07;
                        
                        continue;
                    }
                    
                    double alpha = 1;

                    if (!(scene->flags & RI_SCENE_DONT_USE_AA) || !(mat->flags & RI_MATERIAL_DONT_USE_AA)){
                        double total_inside = 0;
                        
                        for (double sub_y = 1.0 / (-scene->antialiasing_subsample_resolution / 2.0) - 0.5; sub_y < 1.0 / (scene->antialiasing_subsample_resolution / 2.0) - 0.5; sub_y += 1.0 / (scene->antialiasing_subsample_resolution / 2.0)){
                            for (double sub_x = 1.0 / (-scene->antialiasing_subsample_resolution / 2.0) - 0.5; sub_x < 1.0 / (scene->antialiasing_subsample_resolution / 2.0) - 0.5; sub_x += 1.0 / (scene->antialiasing_subsample_resolution / 2.0)){
                                w0 = ((pos_1->y - pos_2->y) * (pixel_x_index + sub_x - pos_2->x) + (pos_2->x - pos_1->x) * (pixel_y_index + sub_y - pos_2->y)) / denominator;
                                w1 = ((pos_2->y - pos_0->y) * (pixel_x_index + sub_x - pos_0->x) + (pos_0->x - pos_2->x) * (pixel_y_index + sub_y - pos_0->y)) / denominator; 
                                w2 = 1.0 - w0 - w1;
                                
                                if(!(w0 >= 0 && w1 >= 0 && w2 >= 0)) total_inside++;
                            }
                        }
                        
                        alpha = 1.0 - total_inside / (scene->antialiasing_subsample_resolution * scene->antialiasing_subsample_resolution);
                    }
                    
                    uint32_t pixel_color = 0x00000000;
         
                    double ux, uy;
                    ux = uy = -1;

                    RI_vector_3f normal = {0};

                    if (normal_0){
                        normal.x = (w0 * (normal_0->x / pos_0->z) + w1 * (normal_1->x / pos_1->z) + w2 * (normal_2->x / pos_2->z)) / w_over_z;
                        normal.y = (w0 * (normal_0->y / pos_0->z) + w1 * (normal_1->y / pos_1->z) + w2 * (normal_2->y / pos_2->z)) / w_over_z;      
                        normal.z = (w0 * (normal_0->z / pos_0->z) + w1 * (normal_1->z / pos_1->z) + w2 * (normal_2->z / pos_2->z)) / w_over_z;      
                    }

                    if (mat->flags & RI_MATERIAL_HAS_TEXTURE && uv_0 && uv_1 && uv_2){                
                        ux = (w0 * (uv_0->x / pos_0->z) + w1 * (uv_1->x / pos_1->z) + w2 * (uv_2->x / pos_2->z)) / w_over_z;
                        uy = (w0 * (uv_0->y / pos_0->z) + w1 * (uv_1->y / pos_1->z) + w2 * (uv_2->y / pos_2->z)) / w_over_z;                
                    
                        if (mat->flags & RI_MATERIAL_USE_UV_LOOP_MULTIPLIER){
                            ux *= mat->uv_loop_multiplier.x;
                            uy *= mat->uv_loop_multiplier.y;
                        }

                        if (mat->flags & RI_MATERIAL_USE_UV_RENDER_RESOLUTION){
                            ux *= (current_face->parent_actor->transform.scale.x / mat->texture_render_size.x);
                            uy *= (current_face->parent_actor->transform.scale.y / mat->texture_render_size.y);
                        }

                        if (mat->flags & RI_MATERIAL_USE_UV_LOOP_MULTIPLIER || mat->flags & RI_MATERIAL_USE_UV_RENDER_RESOLUTION){
                            ux = mod(ux, 1.0);
                            uy = mod(-uy, 1.0);
                        } else{
                            ux = fmax(fmin(ux, 1.0), 0.0);
                            uy = fmax(fmin(1.0 - uy, 1.0), 0.0);
                        }
                        

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

                    if (scene->flags & RI_SCENE_DEBUG_CULLS){ // show unchanged tris in grey, shrunk tris in blue, split triangles in green (old tri) and red (new tri)
                        if (current_face->shrunk) pixel_color = 0xFF7777FF;
                        else if (current_face->split) pixel_color = 0xFF77FF77;
                        else if (face_index >= current_renderable_face_index) pixel_color = 0xFFFF7777;
                        else pixel_color = 0xFF777777;
                    }
                    
                    double shader_result = shader_result = current_face->material_reference->fragment_shader(x, y, *pos_0, *pos_1, *pos_2, normal, (RI_vector_2f){ux, uy}, pixel_color);

                    // set alpha after checking shader result becuase things with alpha 0 should still depth write

                    if (shader_result <= 0) continue;
                    
                    alpha = shader_result;

                    if (!(mat->flags & RI_MATERIAL_DONT_DEPTH_WRITE)){
                        ri.z_buffer[y * target_texture->resolution.x + x] = interpolated_z;
                    }

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

void RI_tick(int clear_window_texture_after_rendering){
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

    if (clear_window_texture_after_rendering){
        RI_wipe_texture(ri.frame_buffer, 0xFF000000);
    }

    ++ri.frame;
}

int opencl_init(){
    cl_int cl_result;

    cl_result = clGetPlatformIDs(1, &ri.cl_platform, &ri.cl_number_of_platforms);

    if (cl_result != CL_SUCCESS || ri.cl_number_of_platforms == 0) {
        debug("[OpenCL Init] Error! No OpenCL platforms");
        RI_stop(1);
    } 
    
    cl_result = clGetDeviceIDs(ri.cl_platform, CL_DEVICE_TYPE_GPU, 1, &ri.cl_device, &ri.cl_number_of_devices);

    if (cl_result != CL_SUCCESS || ri.cl_number_of_devices == 0) {
        debug("[OpenCL Init] Error! No OpenCL devices");
        RI_stop(1);
    }
    
    ri.cl_context = clCreateContext(NULL, 1, &ri.cl_device, NULL, NULL, &cl_result);
    ri.cl_command_queue = clCreateCommandQueue(ri.cl_context, ri.cl_device, 0, &cl_result);

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

void RI_free_scene(RI_scene *scene){
    if (!scene){
        debug("[RI_free_scene] Cannot free scene, already frung");
        return;
    } 

    if (scene->actors) RI_free(scene->actors);
    if (scene->faces_to_render) RI_free(scene->faces_to_render);
    RI_free(scene);
}

void RI_free_texture(RI_texture *texture){
    if (!texture){
        debug("[RI_free_texture] Cannot free texture, already frung");
        return;
    } 

    if (texture->image_buffer) RI_free(texture->image_buffer);
    RI_free(texture);
}

void RI_free_material(RI_material *material){
    if (!material){
        debug("[RI_free_material] Cannot free material, already frung");
        return;
    } 

    RI_free(material);
}

void RI_free_mesh(RI_mesh *mesh){
    if (!mesh){
        debug("[RI_free_mesh] Cannot free mesh, already frung");
        return;
    } 

    if (mesh->faces) RI_free(mesh->faces);
    if (mesh->normals) RI_free(mesh->normals);
    if (mesh->uvs) RI_free(mesh->uvs);
    if (mesh->vertex_positions) RI_free(mesh->vertex_positions);
    RI_free(mesh);
}

void RI_free_actor(RI_actor *actor){
    if (!actor){
        debug("[RI_free_actor] Cannot free actor, already frung");
        return;
    } 

    RI_free(actor);
}

int RI_stop(int result){
    debug("[Notice] Stopping...");

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

    return 0;
}

void signal_interupt_handler(int signal) {
    debug("Recieved SIGINT");
        
    RI_stop(1);
}

double default_fragment_shader(int pixel_x, int pixel_y, RI_vector_3f v_pos_0, RI_vector_3f v_pos_1, RI_vector_3f v_pos_2, RI_vector_3f normal, RI_vector_2f uv, uint32_t color){
    return 1;
}

void perspective_vertex_shader(RI_vector_3f *v_pos_0, RI_vector_3f *v_pos_1, RI_vector_3f *v_pos_2, double horizontal_fov_factor, double vertical_fov_factor){
    v_pos_0->x = v_pos_0->x / v_pos_0->z * horizontal_fov_factor;
    v_pos_0->y = v_pos_0->y / v_pos_0->z * vertical_fov_factor;
    
    v_pos_1->x = v_pos_1->x / v_pos_1->z * horizontal_fov_factor;
    v_pos_1->y = v_pos_1->y / v_pos_1->z * vertical_fov_factor;

    v_pos_2->x = v_pos_2->x / v_pos_2->z * horizontal_fov_factor;
    v_pos_2->y = v_pos_2->y / v_pos_2->z * vertical_fov_factor;
}

void orthographic_vertex_shader(RI_vector_3f *v_pos_0, RI_vector_3f *v_pos_1, RI_vector_3f *v_pos_2, double horizontal_fov_factor, double vertical_fov_factor){
}

int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title){
    signal(SIGINT, signal_interupt_handler);
    
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

    // opencl_init();

    sdl_init(RI_window_width, RI_window_height, RI_window_title);

    ri.loaded_mesh_count = 0;
    ri.loaded_texture_count = 0;
    ri.actor_count = 0;

    char **error_cube_file = RI_malloc(sizeof(char *));
    
    RI_mesh* error_mesh = RI_request_mesh("objects/unit_cube.obj");

    ri.error_mesh = *error_mesh;

    RI_free(error_mesh);
    RI_free(error_cube_file);

    ri.error_texture.image_buffer = RI_malloc(sizeof(uint32_t));

    ri.error_texture.image_buffer[0] = 0xFFFF00FF;
    ri.error_texture.resolution = (RI_vector_2){1, 1};

    ri.default_fragment_shader = default_fragment_shader;
    ri.default_vertex_shader = perspective_vertex_shader;
    ri.perspective_vertex_shader = perspective_vertex_shader;
    ri.orthographic_vertex_shader = orthographic_vertex_shader;

    ri.error_material.bump_map_reference = NULL;
    ri.error_material.normal_map_reference = NULL;
    ri.error_material.uv_loop_multiplier = (RI_vector_2f){1.0, 1.0};
    ri.error_material.wireframe_width = 0.2;
    ri.error_material.texture_reference = &ri.error_texture;
    ri.error_material.albedo = 0xFFFF00FF;
    ri.error_material.fragment_shader = ri.default_fragment_shader;
    ri.error_material.vertex_shader = ri.default_vertex_shader;
    ri.error_material.flags = RI_MATERIAL_UNLIT | RI_MATERIAL_DONT_DEPTH_TEST | RI_MATERIAL_DONT_RECEIVE_SHADOW | RI_MATERIAL_HAS_TEXTURE | RI_MATERIAL_DOUBLE_SIDED;

    return 0;
}
