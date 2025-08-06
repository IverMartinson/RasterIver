#include <stdio.h>
#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"

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

int RI_request_objects(int RI_number_of_requested_objects, char **filenames){
    int objects_already_loaded_count = ri.loaded_object_count;
    
    ri.loaded_object_count += RI_number_of_requested_objects;

    ri.loaded_objects = realloc(ri.loaded_objects, sizeof(RI_object_data) * ri.loaded_object_count);

    for (int i = 0; i < RI_number_of_requested_objects; i++){
        RI_object_data new_object_data_struct;

        new_object_data_struct.face_count = 0;
        new_object_data_struct.vertex_count = 0;
        new_object_data_struct.normal_count = 0;
        new_object_data_struct.uv_count = 0;
        
        FILE *file = fopen(filenames[i], "r");

        if (!file){
            debug("Error! File \"%s\" not found", filenames[i]);
            RI_stop(1);
        }
        
        char line[256];
        
        while (fgets(line, sizeof(line), file)) {
            if (line[0] == 'f' && line[1] == ' ') { // face
                new_object_data_struct.face_count++;
            }
            else if (line[0] == 'v' && line[1] == ' ') { // vertex
                new_object_data_struct.vertex_count++;
            }
            else if (line[0] == 'v' && line[1] == 'n') { // normal
                new_object_data_struct.normal_count++;
            }
            else if (line[0] == 'v' && line[1] == 't') { // UV
                new_object_data_struct.uv_count++;
            }
        }
        
        fclose(file);
        
        new_object_data_struct.faces = malloc(sizeof(RI_face) * new_object_data_struct.face_count);
        new_object_data_struct.vertecies = malloc(sizeof(RI_vertex) * new_object_data_struct.vertex_count);
        new_object_data_struct.normals = malloc(sizeof(RI_vector_3f) * new_object_data_struct.normal_count);
        new_object_data_struct.uvs = malloc(sizeof(RI_vector_2f) * new_object_data_struct.uv_count);

        file = fopen(filenames[i], "r");
        
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
                        
                        has_uvs = 1;
                    }
                }
                else {
                    has_normals = has_uvs = 1;
                }

                new_object_data_struct.faces[current_face_index].vertex_0 = &new_object_data_struct.vertecies[vertex_0_index - 1];
                new_object_data_struct.faces[current_face_index].vertex_1 = &new_object_data_struct.vertecies[vertex_1_index - 1];
                new_object_data_struct.faces[current_face_index].vertex_2 = &new_object_data_struct.vertecies[vertex_2_index - 1];
            
                ++current_face_index;
            }
            else if (line[0] == 'v' && line[1] == ' ') {
                float x, y, z;
                
                sscanf(line, "v %f %f %f", &x, &y, &z);

                new_object_data_struct.vertecies[current_vertex_index].position.x = x;
                new_object_data_struct.vertecies[current_vertex_index].position.y = y;
                new_object_data_struct.vertecies[current_vertex_index].position.z = z;

                ++current_vertex_index;
            } 
            else if (line[0] == 'v' && line[1] == 'n') {
                float x, y, z;
                
                sscanf(line, "vn %f %f %f", &x, &y, &z);

                new_object_data_struct.normals[current_vertex_index].x = x;
                new_object_data_struct.normals[current_vertex_index].y = y;
                new_object_data_struct.normals[current_vertex_index].z = z;
             
                ++current_normal_index;
            }
            else if (line[0] == 'v' && line[1] == 't') {
                float x, y, z;

                sscanf(line, "vt %f %f %f", &x, &y, &z);

                new_object_data_struct.normals[current_vertex_index].x = x;
                new_object_data_struct.normals[current_vertex_index].y = y;
                // UVS are almost always 2D so we don't need this (the type itself is a vector 2f) -> new_object_data_struct.normals[current_vertex_index].z = z; 
              
                ++current_uv_index;
            } 
        }

        char* loading_object_notice_string;

        if (has_normals && !has_uvs) loading_object_notice_string = "normals";
        else if (!has_normals && has_uvs) loading_object_notice_string = "UVs";
        else if (!has_normals && !has_uvs) loading_object_notice_string = "normals and UVs";
        
        if (!has_normals || !has_uvs) debug("Notice! Object \"%s\" is missing %s", filenames[i], loading_object_notice_string);
        
        fclose(file);

        ri.loaded_objects[objects_already_loaded_count + i] = new_object_data_struct;    
    }

    return 0;
}

int RI_tick(){
    // do rendering stuff
    if (ri.running){
        for (int i = 0; i < ri.window_width; ++i){
            ri.frame_buffer[(ri.frame % ri.window_height) * ri.window_width + i] = 0x0 + ri.frame + i * ri.frame;
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

    ri.prefix = "[RasterIver]";

    return 0;
}

int RI_stop(int result){
    debug("Stopping...");
    
    exit(result);

    return 0;
}