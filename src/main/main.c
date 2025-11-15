// rasteriver.c

#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"
#include "../headers/memory.h"

RI_context context;

#define RI_realloc(__ptr, __size) written_RI_realloc(__ptr, __size, __func__, __LINE__, context)
#define RI_malloc(__size) written_RI_malloc(__size, __func__, __LINE__, context)
#define RI_calloc(__nmemb, __size) written_RI_calloc(__nmemb, __size, __func__, __LINE__, context)
#define RI_free(__ptr) written_RI_free(__ptr, __func__, __LINE__, context)

#define PI 3.14159265359
#define PI2 1.57079632679

void debug(char *string, ...){
    if (!context.should_debug)
        return;

    va_list args;
    va_start(args, string);

    char message[500];

    strcpy(message, context.debug_prefix);

    strcat(message, string);

    vprintf(message, args);
    printf("\n");

    va_end(args);
}

RI_texture *RI_new_texture(int width, int height){
    RI_texture *new_texture = RI_malloc(sizeof(RI_texture));

    new_texture->image_buffer = RI_malloc(sizeof(uint32_t) * width * height);
    new_texture->resolution.x = width;
    new_texture->resolution.y = height;

    return new_texture;
}

RI_material *RI_new_material(){
    RI_material *new_material = RI_malloc(sizeof(RI_material));

    new_material->albedo = 0xFFFF00FF;

    return new_material;
}

RI_actor *RI_new_actor(){
    RI_actor *new_actor = RI_malloc(sizeof(RI_actor));

    if (context.defaults.default_actor){
        *new_actor = *context.defaults.default_actor;
    } else {
        new_actor->position = (RI_vector_3){0, 0, 0};
        new_actor->scale = (RI_vector_3){1, 1, 1};
        new_actor->rotation = (RI_vector_4){1, 0, 0, 0};
        new_actor->active = 1;
    }

    return new_actor;
}

RI_scene *RI_new_scene(){
    RI_scene *new_scene = RI_malloc(sizeof(RI_scene));

    new_scene->camera.FOV = PI2;
    new_scene->camera.max_clip = 100000;
    new_scene->camera.min_clip = 0.01;
    new_scene->camera.position = (RI_vector_3){0, 0, 0};
    new_scene->camera.rotation = (RI_vector_4){1, 0, 0, 0};

    return new_scene;
}

RI_mesh *RI_load_mesh(char *filename, RI_actor *actor){
    int previous_face_count = context.opencl.face_count;
    int previous_vertecies_count = context.opencl.vertex_count;
    int previous_normals_count = context.opencl.normal_count;
    int previous_uvs_count = context.opencl.uv_count;

    FILE *file = fopen(filename, "r");

    if (!file){
        debug("[Mesh Loader] Error! File \"%s\" not found", filename);
        return NULL;
    }
    
    char line[512];
    
    int face_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') { // face
            ++face_count;
            ++context.opencl.face_count;
        }
        else if (line[0] == 'v'){
            if (line[1] == ' ') { // vertex
                ++context.opencl.vertex_count;
            }
            else if (line[1] == 'n') { // normal
                ++context.opencl.normal_count;
            }
            else if (line[1] == 't') { // UV
                ++context.opencl.uv_count;
            }
        }
    }

    rewind(file);

    context.opencl.faces = RI_realloc(context.opencl.faces, sizeof(RI_face) * context.opencl.face_count);
    context.opencl.vertecies = RI_realloc(context.opencl.vertecies, sizeof(RI_vector_3) * context.opencl.vertex_count);
    context.opencl.normals = RI_realloc(context.opencl.normals, sizeof(RI_vector_3) * context.opencl.normal_count);
    context.opencl.uvs = RI_realloc(context.opencl.uvs, sizeof(RI_vector_2) * context.opencl.uv_count);

    int current_face_index = previous_face_count;
    int current_vertex_index = previous_vertecies_count;
    int current_normal_index = previous_normals_count;
    int current_uv_index = previous_uvs_count;

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

            context.opencl.faces[current_face_index].position_0_index = vertex_0_index - 1 + previous_vertecies_count;
            context.opencl.faces[current_face_index].position_1_index = vertex_1_index - 1 + previous_vertecies_count;
            context.opencl.faces[current_face_index].position_2_index = vertex_2_index - 1 + previous_vertecies_count;

            context.opencl.faces[current_face_index].normal_0_index = normal_0_index - 1 + previous_normals_count;
            context.opencl.faces[current_face_index].normal_1_index = normal_1_index - 1 + previous_normals_count;
            context.opencl.faces[current_face_index].normal_2_index = normal_2_index - 1 + previous_normals_count;
            
            context.opencl.faces[current_face_index].uv_0_index = uv_0_index - 1 + previous_uvs_count;
            context.opencl.faces[current_face_index].uv_1_index = uv_1_index - 1 + previous_uvs_count;
            context.opencl.faces[current_face_index].uv_2_index = uv_2_index - 1 + previous_uvs_count;

            context.opencl.faces[current_face_index].should_render = 1;

            ++current_face_index;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);

            context.opencl.vertecies[current_vertex_index].x = x;
            context.opencl.vertecies[current_vertex_index].y = y;
            context.opencl.vertecies[current_vertex_index].z = z;

            ++current_vertex_index;
        } 
        else if (line[0] == 'v' && line[1] == 'n') {
            double x, y, z;
            
            sscanf(line, "vn %lf %lf %lf", &x, &y, &z);

            context.opencl.normals[current_normal_index].x = x;
            context.opencl.normals[current_normal_index].y = y;
            context.opencl.normals[current_normal_index].z = z;

            ++current_normal_index;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            double x, y, z;

            sscanf(line, "vt %lf %lf %lf", &x, &y, &z);

            context.opencl.uvs[current_uv_index].x = x;
            context.opencl.uvs[current_uv_index].y = y;
            // UVS are almost always 2D so we don't need Z (the type itself is a vector 2f, not 3f) 

            ++current_uv_index;
        } 
    }

    char* loading_mesh_notice_string;

    if (has_normals && !has_uvs) loading_mesh_notice_string = "normals";
    else if (!has_normals && has_uvs) loading_mesh_notice_string = "UVs";
    else if (!has_normals && !has_uvs) loading_mesh_notice_string = "normals and UVs";
    
    if (!has_normals || !has_uvs) debug("[Mesh Loader] Notice! Mesh \"%s\" is missing %s", filename, loading_mesh_notice_string);
    
    actor->has_normals = has_normals;
    actor->has_uvs = has_uvs;

    actor->face_count = context.opencl.face_count - previous_face_count;
    actor->face_index = previous_face_count;

    debug("[Mesh Loader] Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", filename, current_face_index, current_vertex_index, current_normal_index, current_uv_index); 

    clFinish(context.opencl.queue);

    if (previous_face_count != context.opencl.face_count) {
        if (context.opencl.faces_mem_buffer) clReleaseMemObject(context.opencl.faces_mem_buffer);

        context.opencl.faces_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_face) * context.opencl.face_count, NULL, NULL);
        
        clEnqueueWriteBuffer(context.opencl.queue, context.opencl.faces_mem_buffer, CL_TRUE, 0, sizeof(RI_face) * context.opencl.face_count, context.opencl.faces, 0, NULL, NULL);

        clSetKernelArg(context.opencl.transformation_kernel, 0, sizeof(cl_mem), &context.opencl.faces_mem_buffer);
    
        if (context.opencl.face_count * 2 > context.opencl.length_of_renderable_faces_array){
            debug("old renderable faces count (%d) less than current (%d). Reallocating...", context.opencl.length_of_renderable_faces_array, context.opencl.face_count * 2);

            context.opencl.length_of_renderable_faces_array = context.opencl.face_count * 2;
            
            debug("reallocating %f mb", sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array / 1048576.0);

            context.opencl.faces_to_render = RI_malloc(sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array);

            cl_int error;

            error = clReleaseMemObject(context.opencl.renderable_faces_mem_buffer);

            if (error != CL_SUCCESS){
                debug("couldn't free renderable faces memory buffer (error %d)", error);
                
                exit(1);
            }

            context.opencl.renderable_faces_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array, NULL, &error);
    
            if (error != CL_SUCCESS){
                debug("couldn't reallocate renderable faces memory buffer (error %d)", error);

                exit(1);
            }
        }
    }

    if (previous_vertecies_count != context.opencl.vertex_count) {
        if (context.opencl.vertecies_mem_buffer) clReleaseMemObject(context.opencl.vertecies_mem_buffer);

        context.opencl.vertecies_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_vector_3) * context.opencl.vertex_count, NULL, NULL);
        
        clEnqueueWriteBuffer(context.opencl.queue, context.opencl.vertecies_mem_buffer, CL_TRUE, 0, sizeof(RI_vector_3) * context.opencl.vertex_count, context.opencl.vertecies, 0, NULL, NULL);

        clSetKernelArg(context.opencl.transformation_kernel, 1, sizeof(cl_mem), &context.opencl.vertecies_mem_buffer);
    }

    if (previous_normals_count != context.opencl.normal_count) {
        if (context.opencl.normals_mem_buffer) clReleaseMemObject(context.opencl.normals_mem_buffer);

        context.opencl.normals_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_vector_3) * context.opencl.normal_count, NULL, NULL);
        
        clEnqueueWriteBuffer(context.opencl.queue, context.opencl.normals_mem_buffer, CL_TRUE, 0, sizeof(RI_vector_3) * context.opencl.normal_count, context.opencl.normals, 0, NULL, NULL);

        clSetKernelArg(context.opencl.transformation_kernel, 2, sizeof(cl_mem), &context.opencl.normals_mem_buffer);
    }

    if (previous_uvs_count != context.opencl.uv_count) {
        if (context.opencl.uvs_mem_buffer) clReleaseMemObject(context.opencl.uvs_mem_buffer);

        context.opencl.uvs_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_vector_2) * context.opencl.uv_count, NULL, NULL);
        
        clEnqueueWriteBuffer(context.opencl.queue, context.opencl.uvs_mem_buffer, CL_TRUE, 0, sizeof(RI_vector_2) * context.opencl.uv_count, context.opencl.uvs, 0, NULL, NULL);

        clSetKernelArg(context.opencl.transformation_kernel, 3, sizeof(cl_mem), &context.opencl.uvs_mem_buffer);
    }

    fclose(file);
}

void RI_render(RI_texture *target_texture, RI_scene *scene){
    if (!target_texture){
        target_texture = context.sdl.frame_buffer;
    }

    // transformer 
    
    double horizontal_fov_factor = (double)target_texture->resolution.x / tanf(0.5 * scene->camera.FOV);
    double vertical_fov_factor = (double)target_texture->resolution.y / tanf(0.5 * scene->camera.FOV);
    

    // kernel args    
    
    // 21, double horizontal_fov_factor
    clSetKernelArg(context.opencl.transformation_kernel, 21, sizeof(double), &horizontal_fov_factor);
    // 22, double vertical_fov_factor
    clSetKernelArg(context.opencl.transformation_kernel, 22, sizeof(double), &vertical_fov_factor);

    // 23, double min_clip
    clSetKernelArg(context.opencl.transformation_kernel, 23, sizeof(float), &scene->camera.min_clip);
    // 24, double max_clip
    clSetKernelArg(context.opencl.transformation_kernel, 24, sizeof(float), &scene->camera.max_clip);

    // 25, double camera_x
    clSetKernelArg(context.opencl.transformation_kernel, 25, sizeof(double), &scene->camera.position.x);
    // 26, double camera_y
    clSetKernelArg(context.opencl.transformation_kernel, 26, sizeof(double), &scene->camera.position.y);
    // 27, double camera_z
    clSetKernelArg(context.opencl.transformation_kernel, 27, sizeof(double), &scene->camera.position.z);

    // 28, double camera_r_w
    clSetKernelArg(context.opencl.transformation_kernel, 28, sizeof(double), &scene->camera.rotation.w);
    // 29, double camera_r_x
    clSetKernelArg(context.opencl.transformation_kernel, 29, sizeof(double), &scene->camera.rotation.x);
    // 30, double camera_r_y
    clSetKernelArg(context.opencl.transformation_kernel, 30, sizeof(double), &scene->camera.rotation.y);
    // 31, double camera_r_z
    clSetKernelArg(context.opencl.transformation_kernel, 31, sizeof(double), &scene->camera.rotation.z);


    int local_group_size_x = 16;
    int local_group_size_y = 16;

    // count faces
    scene->face_count = 0;
    for (int actor_index = 0; actor_index < scene->length_of_actors_array; ++actor_index){
        scene->face_count += scene->actors[actor_index]->face_count;
    }


    // allocate faces_to_render if face count increases
    if (scene->face_count > context.opencl.length_of_renderable_faces_array){
        context.opencl.faces_to_render = RI_realloc(context.opencl.faces_to_render, sizeof(RI_renderable_face) * scene->face_count * 2); // x2 because faces can be split
    
        context.opencl.length_of_renderable_faces_array = scene->face_count;
    }

    
    // set faces_to_render to zero
    memset(context.opencl.faces_to_render, 0, sizeof(RI_renderable_face) * scene->face_count * 2);

    clSetKernelArg(context.opencl.transformation_kernel, 4, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);

    context.current_renderable_face_index = 0;
    context.current_split_renderable_face_index = 0;

    debug("transforming polygons...");

    int renderable_face_index = 0;

    cl_event event;
    cl_ulong start, end;

    // transform polygons
    for (int actor_index = 0; actor_index < scene->length_of_actors_array; ++actor_index){
        RI_actor *actor = scene->actors[actor_index];
        
        debug("actor index: %d face count: %d", actor_index, actor->face_count);

        if (scene->actors[actor_index]->face_count <= 0) continue;
        
        int face_sqrt = ceil(sqrt(scene->actors[actor_index]->face_count));

        int local_t_size = (int)fmin(face_sqrt, local_group_size_x);

        const size_t t_global_work_size[2] = {local_t_size * ceil(face_sqrt / (float)local_t_size), local_t_size * ceil(face_sqrt / (float)local_t_size)};
        const size_t t_local_work_size[2] = {(int)fmin(face_sqrt, local_group_size_x), (int)fmin(face_sqrt, local_group_size_y)};

        debug("transformer global work size: {%d, %d}", t_global_work_size[0], t_global_work_size[1]);    
        debug("transformer local work size: {%d, %d}", t_local_work_size[0], t_local_work_size[1]);

        debug("(%d extra work items; %d items (%dx%d) - %d faces)", t_global_work_size[0] * t_global_work_size[1] - scene->actors[actor_index]->face_count, t_global_work_size[0] * t_global_work_size[1], t_global_work_size[0], t_global_work_size[1], scene->actors[actor_index]->face_count);

        // 5, double actor_x
        clSetKernelArg(context.opencl.transformation_kernel, 5, sizeof(double), &actor->position.x);
        // 6, double actor_y
        clSetKernelArg(context.opencl.transformation_kernel, 6, sizeof(double), &actor->position.y);
        // 7, double actor_z
        clSetKernelArg(context.opencl.transformation_kernel, 7, sizeof(double), &actor->position.z);

        // 8, double actor_r_w
        clSetKernelArg(context.opencl.transformation_kernel, 8, sizeof(double), &actor->rotation.w);
        // 9, double actor_r_x
        clSetKernelArg(context.opencl.transformation_kernel, 9, sizeof(double), &actor->rotation.x);
        // 10, double actor_r_y
        clSetKernelArg(context.opencl.transformation_kernel, 10, sizeof(double), &actor->rotation.y);
        // 11, double actor_r_z
        clSetKernelArg(context.opencl.transformation_kernel, 11, sizeof(double), &actor->rotation.z);

        // 12, double actor_s_x
        clSetKernelArg(context.opencl.transformation_kernel, 12, sizeof(double), &actor->scale.x);
        // 13, double actor_s_y
        clSetKernelArg(context.opencl.transformation_kernel, 13, sizeof(double), &actor->scale.y);
        // 14, double actor_s_z
        clSetKernelArg(context.opencl.transformation_kernel, 14, sizeof(double), &actor->scale.z);

        // 15, int has_normals
        clSetKernelArg(context.opencl.transformation_kernel, 15, sizeof(int), &actor->has_normals);
        // 16, int has_uvs
        clSetKernelArg(context.opencl.transformation_kernel, 16, sizeof(int), &actor->has_uvs);
        // 17, int face_array_offset_index
        clSetKernelArg(context.opencl.transformation_kernel, 17, sizeof(int), &actor->face_index);
        // 18, int face_count
        clSetKernelArg(context.opencl.transformation_kernel, 18, sizeof(int), &actor->face_count);

        // 32, int renderable_face_offset
        clSetKernelArg(context.opencl.transformation_kernel, 32, sizeof(int), &renderable_face_index);
        
        // 33, int face_sqrt
        clSetKernelArg(context.opencl.transformation_kernel, 33, sizeof(int), &face_sqrt);

        debug("running kernel...");

        clEnqueueNDRangeKernel(context.opencl.queue, context.opencl.transformation_kernel, 2, NULL, t_global_work_size, t_local_work_size, 0, NULL, &event);
        clFinish(context.opencl.queue);

        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);

        double ns = (double)(end - start);
        printf("actor #%d's transformation kernel: %f ms\n", actor_index, ns / 1e6);
        
        debug("done");
    
        renderable_face_index += actor->face_count * 2;
    }

    debug("done");    
    

    // rasterize

    // set width, height and kernel width, height
    int x = context.window.width;
    int y = context.window.height;

    int x_div_32 = ceil(context.window.width / (float)local_group_size_x);
    int y_div_32 = ceil(context.window.height / (float)local_group_size_y);

    if (context.window.width % local_group_size_x != 0)
        x = local_group_size_x * x_div_32;

    if (context.window.height % local_group_size_y != 0)
        y = local_group_size_y * y_div_32;

    const size_t r_global_work_size[2] = {x, y};
    const size_t r_local_work_size[2] = {local_group_size_x, local_group_size_y};

    debug("rasterizer global work size: {%d, %d}", x, y);    
    debug("rasterizer local work size: {%d, %d}", local_group_size_x, local_group_size_y);    

    // kernel args
    clSetKernelArg(context.opencl.rasterization_kernel, 0, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 6, sizeof(int), &scene->face_count);
    clSetKernelArg(context.opencl.rasterization_kernel, 7, sizeof(int), &context.current_split_renderable_face_index);

    debug("rasterizing...");

    // run raster kernel
    clEnqueueNDRangeKernel(context.opencl.queue, context.opencl.rasterization_kernel, 2, NULL, r_global_work_size, r_local_work_size, 0, NULL, NULL);
    clFinish(context.opencl.queue);
    
    debug("done\ncopying frame buffer...");

    // put GPU frame buffer into CPU
    clEnqueueReadBuffer(context.opencl.queue, context.opencl.frame_buffer_mem_buffer, CL_TRUE, 0, context.window.width * context.window.height * sizeof(uint32_t), context.sdl.frame_buffer->image_buffer, 0, NULL, &event);
    clFinish(context.opencl.queue);

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);

    double ns = (double)(end - start);
    printf("rasterization kernel: %f ms\n", ns / 1e6);

    debug("done");
}

void RI_tick(){
    SDL_Event event;

    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT: {
                context.is_running = ri_false;

                break;
            }

            default: {
                break;
            }
        }
    }

    SDL_LockTexture(context.sdl.frame_buffer_texture, NULL, (void*)&context.sdl.frame_buffer_intermediate, &context.sdl.pitch);

    memcpy(context.sdl.frame_buffer_intermediate, context.sdl.frame_buffer->image_buffer, context.window.width * context.window.height * sizeof(uint32_t));

    SDL_UnlockTexture(context.sdl.frame_buffer_texture);

    SDL_RenderCopy(context.sdl.renderer, context.sdl.frame_buffer_texture, NULL, NULL);
    SDL_RenderPresent(context.sdl.renderer);

    ++context.current_frame;

    return;
}

RI_context *RI_get_context(){
    context.sdl = (RI_SDL){NULL, NULL, NULL, NULL, 0};
    context.window = (RI_window){800, 800, 400, 400, "RasterIver Window"};

    return &context;
} 

// Convert a CL file to a string
char *load_kernel_source(const char *filename) {
    FILE *f = fopen(filename, "rb");

    if (f == NULL){
        debug("couldn't open kernel file \"%s\"", filename);
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    return source;
}

int RI_init(){
    context.window.half_width = context.window.width / 2;
    context.window.half_height = context.window.height / 2;

    context.current_frame = 0;

    // init SDL
    context.sdl.window = SDL_CreateWindow(context.window.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, context.window.width, context.window.height, 0);
    context.sdl.renderer = SDL_CreateRenderer(context.sdl.window, -1, SDL_RENDERER_ACCELERATED);
    context.sdl.frame_buffer_texture = SDL_CreateTexture(context.sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, context.window.width, context.window.height);

    context.sdl.frame_buffer_intermediate = malloc(sizeof(uint32_t) * context.window.width * context.window.height);
    context.sdl.frame_buffer = RI_new_texture(context.window.width, context.window.height);

    context.is_running = ri_true;
    context.should_debug = ri_true;

    context.debug_prefix = "[RasterIver] ";

    context.defaults.default_actor = RI_new_actor();
    
    // init OpenCL
 
    context.opencl.length_of_renderable_faces_array = 1;
    context.opencl.face_count = 0;
    context.opencl.vertex_count = 0;
    context.opencl.normal_count = 0;
    context.opencl.uv_count = 0;
    
    context.opencl.faces_to_render = RI_malloc(sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array);
    context.opencl.faces = RI_malloc(sizeof(RI_face) * context.opencl.face_count);

    cl_int error;

    cl_uint num_platforms = 0;
    error = clGetPlatformIDs(0, NULL, &num_platforms);

    cl_platform_id *platforms = malloc(sizeof(cl_platform_id) * num_platforms);
    error = clGetPlatformIDs(num_platforms, platforms, NULL);

    cl_device_id *devices; 
    
    cl_platform_id chosen_platform = NULL;
    char pname[256];
    for (cl_uint i = 0; i < num_platforms; i++) {
        error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(pname), pname, NULL);

        debug("get platform info result: %d", error);

        cl_uint num_devices = 0;
        error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

        debug("num devices result: %d", error);

        devices = malloc(sizeof(cl_device_id) * num_devices);
        error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

        if (i == 0){
            context.opencl.device = devices[0];
        }

        debug("get device ids result: %d", error);

        debug("-platform #%d: NAME: %s | DEVICE COUNT: %d", i, pname, num_devices);

        for (int j = 0; j < num_devices; ++j){
            debug("-\\ device #%d: ID: %u", j, devices[j]);
        }

        free(devices);
    }

    context.opencl.platform = platforms[0];

    debug("device id: %u", context.opencl.device);

    context.opencl.context = clCreateContext(NULL, 1, &context.opencl.device, NULL, NULL, NULL);
    context.opencl.queue = clCreateCommandQueue(context.opencl.context, context.opencl.device, CL_QUEUE_PROFILING_ENABLE, &error);

    if (!context.opencl.context){
        debug("failed to create OpenCL context");
    
        exit(1);
    }

    // build programs

    char *program_source = load_kernel_source("src/kernels/kernels.cl");    
    cl_program rasterization_program = clCreateProgramWithSource(context.opencl.context, 1, (const char**)&program_source, NULL, NULL);
    free(program_source);

    cl_int result = clBuildProgram(rasterization_program, 1, &context.opencl.device, "", NULL, NULL);

    if (result != CL_SUCCESS){
        char log[256];

        clGetProgramBuildInfo(rasterization_program, context.opencl.device, CL_PROGRAM_BUILD_LOG, 10000, log, NULL);

        debug("rasterization program build failed (%d). Log: \n  %s", result, log);
    
        return 1;
    }

    // kernels

    context.opencl.rasterization_kernel = clCreateKernel(rasterization_program, "rasterizer", &error);

    if (error != CL_SUCCESS){
        debug("couldn't create rasterizer kernel");
        return 1;
    }

    context.opencl.transformation_kernel = clCreateKernel(rasterization_program, "transformer", &error);

    if (error != CL_SUCCESS){
        debug("couldn't create transformer kernel");
        return 1;
    }

    // rasterizer

    context.opencl.renderable_faces_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array, NULL, &error);
    
    if (error != CL_SUCCESS){
        debug("couldn't create renderable faces memory buffer");
        return 1;
    }
    
    context.opencl.frame_buffer_mem_buffer = clCreateBuffer(context.opencl.context, CL_MEM_READ_WRITE, sizeof(uint32_t) * context.window.width * context.window.height, NULL, &error);

    if (error != CL_SUCCESS || !context.opencl.frame_buffer_mem_buffer){
        debug("couldn't create frame buffer memory buffer");
        return 1;
    }

// rasterizer(__global RI_renderable_face *renderable_faces, __global uint *frame_buffer, 
//            int width, int height, int half_width, int half_height, int number_of_renderable_faces, 
//            int number_of_split_renderable_faces)

    clSetKernelArg(context.opencl.rasterization_kernel, 0, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 1, sizeof(cl_mem), &context.opencl.frame_buffer_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 2, sizeof(int), &context.window.width);
    clSetKernelArg(context.opencl.rasterization_kernel, 3, sizeof(int), &context.window.height);
    clSetKernelArg(context.opencl.rasterization_kernel, 4, sizeof(int), &context.window.half_width);
    clSetKernelArg(context.opencl.rasterization_kernel, 5, sizeof(int), &context.window.half_height);
    clSetKernelArg(context.opencl.rasterization_kernel, 6, sizeof(int), &context.current_renderable_face_index);
    clSetKernelArg(context.opencl.rasterization_kernel, 7, sizeof(int), &context.current_split_renderable_face_index);

    // transformer

    context.opencl.faces_mem_buffer = NULL;
    context.opencl.vertecies_mem_buffer = NULL;
    context.opencl.normals_mem_buffer = NULL;
    context.opencl.uvs_mem_buffer = NULL;

    // transformer

    // __global RI_face *faces, __global RI_vector_3 *vertecies, 
    // __global RI_vector_3 *normals, __global RI_vector_2 *uvs, 
    // __global RI_renderable_face *renderable_faces, 
    // double actor_x, double actor_y, double actor_z, 
    // double actor_r_w, double actor_r_x, double actor_r_y, double actor_r_z, 
    // double actor_s_x, double actor_s_y, double actor_s_z, 
    // int has_normals, int has_uvs, int face_array_offset_index, int face_count, 
    // int width, int height, double horizontal_fov_factor, double vertical_fov_factor, 
    // double min_clip, double max_clip, 
    // double camera_x, double camera_y, double camera_z, 
    // double camera_r_w, double camera_r_x, double camera_r_y, double camera_r_z

    // // 0, __global RI_face *faces
    // clSetKernelArg(context.opencl.transformation_kernel, 0, sizeof(cl_mem), &context.opencl.faces_mem_buffer);
    // // 1, __global RI_vector_3 *vertecies
    // clSetKernelArg(context.opencl.transformation_kernel, 1, sizeof(cl_mem), &context.opencl.vertecies_mem_buffer);
    // // 2, __global RI_vector_3 *normals
    // clSetKernelArg(context.opencl.transformation_kernel, 2, sizeof(cl_mem), &context.opencl.normals_mem_buffer);
    // // 3, __global RI_vector_2 *uvs
    // clSetKernelArg(context.opencl.transformation_kernel, 3, sizeof(cl_mem), &context.opencl.uvs_mem_buffer);
    // // 4, __global RI_renderable_face *renderable_faces
    // clSetKernelArg(context.opencl.transformation_kernel, 4, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);

    // // 5, double actor_x
    // clSetKernelArg(context.opencl.transformation_kernel, 5, sizeof(double), &actor_x);
    // // 6, double actor_y
    // clSetKernelArg(context.opencl.transformation_kernel, 6, sizeof(double), &actor_y);
    // // 7, double actor_z
    // clSetKernelArg(context.opencl.transformation_kernel, 7, sizeof(double), &actor_z);

    // // 8, double actor_r_w
    // clSetKernelArg(context.opencl.transformation_kernel, 8, sizeof(double), &actor_r_w);
    // // 9, double actor_r_x
    // clSetKernelArg(context.opencl.transformation_kernel, 9, sizeof(double), &actor_r_x);
    // // 10, double actor_r_y
    // clSetKernelArg(context.opencl.transformation_kernel, 10, sizeof(double), &actor_r_y);
    // // 11, double actor_r_z
    // clSetKernelArg(context.opencl.transformation_kernel, 11, sizeof(double), &actor_r_z);

    // // 12, double actor_s_x
    // clSetKernelArg(context.opencl.transformation_kernel, 12, sizeof(double), &actor_s_x);
    // // 13, double actor_s_y
    // clSetKernelArg(context.opencl.transformation_kernel, 13, sizeof(double), &actor_s_y);
    // // 14, double actor_s_z
    // clSetKernelArg(context.opencl.transformation_kernel, 14, sizeof(double), &actor_s_z);

    // // 15, int has_normals
    // clSetKernelArg(context.opencl.transformation_kernel, 15, sizeof(int), &has_normals);
    // // 16, int has_uvs
    // clSetKernelArg(context.opencl.transformation_kernel, 16, sizeof(int), &has_uvs);
    // // 17, int face_array_offset_index
    // clSetKernelArg(context.opencl.transformation_kernel, 17, sizeof(int), &face_array_offset_index);
    // // 18, int face_count
    // clSetKernelArg(context.opencl.transformation_kernel, 18, sizeof(int), &face_count);

    // // 19, int width
    clSetKernelArg(context.opencl.transformation_kernel, 19, sizeof(int), &context.window.width);
    // // 20, int height
    clSetKernelArg(context.opencl.transformation_kernel, 20, sizeof(int), &context.window.height);

    // // 21, double horizontal_fov_factor
    // clSetKernelArg(context.opencl.transformation_kernel, 21, sizeof(double), &horizontal_fov_factor);
    // // 22, double vertical_fov_factor
    // clSetKernelArg(context.opencl.transformation_kernel, 22, sizeof(double), &vertical_fov_factor);

    // // 23, double min_clip
    // clSetKernelArg(context.opencl.transformation_kernel, 23, sizeof(double), &min_clip);
    // // 24, double max_clip
    // clSetKernelArg(context.opencl.transformation_kernel, 24, sizeof(double), &max_clip);

    // // 25, double camera_x
    // clSetKernelArg(context.opencl.transformation_kernel, 25, sizeof(double), &camera_x);
    // // 26, double camera_y
    // clSetKernelArg(context.opencl.transformation_kernel, 26, sizeof(double), &camera_y);
    // // 27, double camera_z
    // clSetKernelArg(context.opencl.transformation_kernel, 27, sizeof(double), &camera_z);

    // // 28, double camera_r_w
    // clSetKernelArg(context.opencl.transformation_kernel, 28, sizeof(double), &camera_r_w);
    // // 29, double camera_r_x
    // clSetKernelArg(context.opencl.transformation_kernel, 29, sizeof(double), &camera_r_x);
    // // 30, double camera_r_y
    // clSetKernelArg(context.opencl.transformation_kernel, 30, sizeof(double), &camera_r_y);
    // // 31, double camera_r_z
    // clSetKernelArg(context.opencl.transformation_kernel, 31, sizeof(double), &camera_r_z);
    
    // // 32, int renderable_face_offset
    // clSetKernelArg(context.opencl.transformation_kernel, 32, sizeof(int), &renderable_face_offset);
    // // 33, int face_sqrt
    // clSetKernelArg(context.opencl.transformation_kernel, 33, sizeof(int), &face_sqrt);

    RI_load_mesh("objects/cube.obj", context.defaults.default_actor);


    return 0;
}