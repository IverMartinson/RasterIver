// rasteriver.c

#include <CL/cl.h>
#include <SDL2/SDL.h>
#include "../headers/rasteriver.h"
#include "../headers/memory.h"
#include "../headers/pitmap.h"
#include <time.h>

cl_int clerror;
cl_event clevent;

RI_context context;

#define RI_realloc(__ptr, __size) written_RI_realloc(__ptr, __size, __func__, __LINE__, context)
#define RI_malloc(__size) written_RI_malloc(__size, __func__, __LINE__, context)
#define RI_calloc(__nmemb, __size) written_RI_calloc(__nmemb, __size, __func__, __LINE__, context)
#define RI_free(__ptr) written_RI_free(__ptr, __func__, __LINE__, context)

#define PI 3.14159265359
#define PI2 1.57079632679

void debug(char *string, int debug_flag, ...){
    if (!(context.debug_flags & debug_flag))
        return;

    va_list args;
    va_start(args, debug_flag);

    char message[500];

    strcpy(message, context.debug_prefix);

    strcat(message, string);

    vprintf(message, args);
    printf("\n");

    va_end(args);
}

RI_texture* RI_load_image(char* filename){
    PM_image* image = PM_load_image(filename);

    RI_texture* texture = RI_malloc(sizeof(RI_texture));

    int previous_length_of_textures_array = context.opencl.length_of_textures_array;

    texture->width = image->width;
    texture->height = image->height;
    texture->index = previous_length_of_textures_array;

    context.opencl.length_of_textures_array += image->width * image->height;

    context.opencl.textures = RI_realloc(context.opencl.textures, context.opencl.length_of_textures_array * sizeof(uint32_t));

    memcpy(context.opencl.textures + previous_length_of_textures_array, image->frame_buffer, sizeof(uint32_t) * image->width * image->height);

    if (context.opencl.textures_mem_buffer) clReleaseMemObject(context.opencl.textures_mem_buffer);

    context.opencl.textures_mem_buffer = clCreateBuffer(
        context.opencl.context, 
        CL_MEM_READ_WRITE, 
        sizeof(uint32_t) * context.opencl.length_of_textures_array, 
        NULL, NULL
    );
    
    clEnqueueWriteBuffer(
        context.opencl.queue, 
        context.opencl.textures_mem_buffer, 
        CL_TRUE, 
        0, 
        sizeof(uint32_t) * context.opencl.length_of_textures_array, 
        context.opencl.textures, 
        0, NULL, NULL
    );
    
    clFinish(context.opencl.queue);

    clSetKernelArg(
        context.opencl.rasterization_kernel, 
        1, 
        sizeof(cl_mem), 
        &context.opencl.textures_mem_buffer
    );

    free(image->frame_buffer);
    free(image);

    return texture;
}

RI_material *RI_new_material(){
    RI_material *new_material = RI_malloc(sizeof(RI_material));

    new_material->albedo = 0xFFFF00FF;

    return new_material;
}

RI_actor *RI_new_actor(){
    RI_actor *new_actor = RI_malloc(sizeof(RI_actor));

    *new_actor = *context.defaults.default_actor;

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

RI_mesh *RI_load_mesh(char *filename){
    clock_t start_time, end_time;
    
    start_time = clock();

    RI_mesh *new_mesh = RI_malloc(sizeof(RI_mesh));

    int previous_face_count = context.opencl.face_count;
    int previous_vertecies_count = context.opencl.vertex_count;
    int previous_normals_count = context.opencl.normal_count;
    int previous_uvs_count = context.opencl.uv_count;

    FILE *file = fopen(filename, "r");

    if (!file){
        debug("[Mesh Loader] Error! File \"%s\" not found", RI_DEBUG_MESH_LOADER_ERROR, filename);
        return NULL;
    }
    
    char line[512];
    
    int face_count = 0;

    int object_face_count = 0;
    int object_vertecies_count = 0;
    int object_normals_count = 0;
    int object_uvs_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') { // face
            ++face_count;
            ++context.opencl.face_count;
            ++object_face_count;
        }
        else if (line[0] == 'v'){
            if (line[1] == ' ') { // vertex
                ++context.opencl.vertex_count;
                ++object_vertecies_count;
            }
            else if (line[1] == 'n') { // normal
                ++context.opencl.normal_count;
                ++object_normals_count;
            }
            else if (line[1] == 't') { // UV
                ++context.opencl.uv_count;
                ++object_uvs_count;
            }
        }
    }

    rewind(file);

    debug(
        "%d faces %d vertecies %d normals %d uvs", 
        RI_DEBUG_MESH_LOADER_FACE_VERT_NORM_UV_COUNT,
        object_face_count, 
        object_vertecies_count, 
        object_normals_count, 
        object_uvs_count
    );

    context.opencl.faces = RI_realloc(context.opencl.faces, sizeof(RI_face) * context.opencl.face_count);
    context.opencl.temp_faces = RI_malloc(sizeof(RI_face) * object_face_count);
    context.opencl.temp_vertecies = RI_malloc(sizeof(RI_vector_3) * object_vertecies_count);
    if (object_normals_count > 0) context.opencl.temp_normals = RI_malloc(
        sizeof(RI_vector_3) * object_normals_count
    );
    if (object_uvs_count > 0) context.opencl.temp_uvs = RI_malloc(sizeof(RI_vector_2) * object_uvs_count);

    int current_face_index = 0;
    int current_vertex_index = 0;
    int current_normal_index = 0;
    int current_uv_index = 0;

    int has_normals, has_uvs;
    has_normals = has_uvs = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') {
            int vertex_0_index, 
                vertex_1_index, 
                vertex_2_index, 
                normal_0_index, 
                normal_1_index, 
                normal_2_index, 
                uv_0_index, 
                uv_1_index, 
                uv_2_index
            ;

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

            context.opencl.temp_faces[current_face_index].position_0_index = vertex_0_index - 1;
            context.opencl.temp_faces[current_face_index].position_1_index = vertex_1_index - 1;
            context.opencl.temp_faces[current_face_index].position_2_index = vertex_2_index - 1;

            context.opencl.temp_faces[current_face_index].normal_0_index = normal_0_index - 1;
            context.opencl.temp_faces[current_face_index].normal_1_index = normal_1_index - 1;
            context.opencl.temp_faces[current_face_index].normal_2_index = normal_2_index - 1;
            
            context.opencl.temp_faces[current_face_index].uv_0_index = uv_0_index - 1;
            context.opencl.temp_faces[current_face_index].uv_1_index = uv_1_index - 1;
            context.opencl.temp_faces[current_face_index].uv_2_index = uv_2_index - 1;

            context.opencl.faces[current_face_index].should_render = 1;

            ++current_face_index;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);

            context.opencl.temp_vertecies[current_vertex_index].x = x;
            context.opencl.temp_vertecies[current_vertex_index].y = y;
            context.opencl.temp_vertecies[current_vertex_index].z = z;

            ++current_vertex_index;
        } 
        else if (line[0] == 'v' && line[1] == 'n') {
            double x, y, z;
            
            sscanf(line, "vn %lf %lf %lf", &x, &y, &z);

            context.opencl.temp_normals[current_normal_index].x = x;
            context.opencl.temp_normals[current_normal_index].y = y;
            context.opencl.temp_normals[current_normal_index].z = z;

            ++current_normal_index;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            double x, y, z;

            sscanf(line, "vt %lf %lf %lf", &x, &y, &z);

            context.opencl.temp_uvs[current_uv_index].x = x;
            context.opencl.temp_uvs[current_uv_index].y = y;
            // UVS are almost always 2D so we don't need Z (the type itself is a vector 2f, not 3f) 

            ++current_uv_index;
        } 
    }

    for (int i = 0; i < object_face_count; ++i){
        context.opencl.faces[i + previous_face_count].position_0 = context.opencl.temp_vertecies[context.opencl.temp_faces[i].position_0_index];
        context.opencl.faces[i + previous_face_count].position_1 = context.opencl.temp_vertecies[context.opencl.temp_faces[i].position_1_index];
        context.opencl.faces[i + previous_face_count].position_2 = context.opencl.temp_vertecies[context.opencl.temp_faces[i].position_2_index];

        context.opencl.faces[i + previous_face_count].normal_0 = context.opencl.temp_normals[context.opencl.temp_faces[i].normal_0_index];
        context.opencl.faces[i + previous_face_count].normal_1 = context.opencl.temp_normals[context.opencl.temp_faces[i].normal_1_index];
        context.opencl.faces[i + previous_face_count].normal_2 = context.opencl.temp_normals[context.opencl.temp_faces[i].normal_2_index];

        context.opencl.faces[i + previous_face_count].uv_0 = context.opencl.temp_uvs[context.opencl.temp_faces[i].uv_0_index];
        context.opencl.faces[i + previous_face_count].uv_1 = context.opencl.temp_uvs[context.opencl.temp_faces[i].uv_1_index];
        context.opencl.faces[i + previous_face_count].uv_2 = context.opencl.temp_uvs[context.opencl.temp_faces[i].uv_2_index];
    }

    free(context.opencl.temp_faces);
    free(context.opencl.temp_vertecies);
    if (object_normals_count > 0) free(context.opencl.temp_normals);
    if (object_uvs_count > 0) free(context.opencl.temp_uvs);

    char* loading_mesh_notice_string;

    if (has_normals && !has_uvs) loading_mesh_notice_string = "normals";
    else if (!has_normals && has_uvs) loading_mesh_notice_string = "UVs";
    else if (!has_normals && !has_uvs) loading_mesh_notice_string = "normals and UVs";
    
    if (!has_normals || !has_uvs) debug(
        "[Mesh Loader] Notice! Mesh \"%s\" is missing %s", 
        RI_DEBUG_MESH_LOADER_ERROR,
        filename, 
        loading_mesh_notice_string
    );
    
    new_mesh->has_normals = has_normals;
    new_mesh->has_uvs = has_uvs;

    new_mesh->face_count = object_face_count;
    new_mesh->face_index = previous_face_count;

    if (object_face_count > context.opencl.lagest_face_count){
        context.opencl.lagest_face_count = object_face_count;
    
        // rebuild tile array
        if (context.opencl.tiles_mem_buffer) clReleaseMemObject(context.opencl.tiles_mem_buffer);

        // # of horizontal tiles
        int num_of_h_tiles = ceil((double)context.window.width / (double)context.opencl.tile_width);
        // # of vertical tiles
        int num_of_v_tiles = ceil((double)context.window.height / (double)context.opencl.tile_height);

        context.opencl.num_h_tiles = num_of_h_tiles;
        
        context.opencl.num_v_tiles = num_of_v_tiles;

        int tile_count = num_of_v_tiles * num_of_h_tiles + num_of_h_tiles;

        // t width, t height, faces per tile,    {faces in this tile, ... faces} <- repeat for # of tiles
        int tile_buffer_size = sizeof(uint32_t) * (5 + tile_count * (object_face_count + 1));

        context.opencl.tiles_mem_buffer = clCreateBuffer(
            context.opencl.context, 
            CL_MEM_READ_WRITE, 
            tile_buffer_size, 
            NULL, NULL
        );

        uint32_t t_info[5] = {context.opencl.tile_width, context.opencl.tile_height, object_face_count, num_of_h_tiles, num_of_v_tiles};

        clEnqueueWriteBuffer(
            context.opencl.queue, 
            context.opencl.tiles_mem_buffer, 
            CL_TRUE, 
            0, 
            sizeof(uint32_t) * 5, 
            t_info, 
            0, NULL, NULL
        );

        clFinish(context.opencl.queue);

        clSetKernelArg(context.opencl.tile_clear_kernel, 0, sizeof(cl_mem), &context.opencl.tiles_mem_buffer);

        clSetKernelArg(context.opencl.rasterization_kernel, 9, sizeof(cl_mem), &context.opencl.tiles_mem_buffer);

        // 34: uint32_t tiles_mem_buffer
        clSetKernelArg(context.opencl.transformation_kernel, 34, sizeof(cl_mem), &context.opencl.tiles_mem_buffer);
    }

    debug(
        "[Mesh Loader] Loaded mesh \"%s\"! %d faces, %d verticies, %d normals, %d uvs", 
        RI_DEBUG_MESH_LOADER_LOADED_MESH,
        filename, 
        current_face_index, 
        current_vertex_index, 
        current_normal_index, 
        current_uv_index
    ); 

    clFinish(context.opencl.queue);

    if (previous_face_count != context.opencl.face_count) {
        if (context.opencl.faces_mem_buffer) clReleaseMemObject(context.opencl.faces_mem_buffer);

        context.opencl.faces_mem_buffer = clCreateBuffer(
            context.opencl.context, 
            CL_MEM_READ_WRITE, 
            sizeof(RI_face) * context.opencl.face_count, 
            NULL, NULL
        );
        
        clEnqueueWriteBuffer(
            context.opencl.queue, 
            context.opencl.faces_mem_buffer, 
            CL_TRUE, 
            0, 
            sizeof(RI_face) * context.opencl.face_count, 
            context.opencl.faces, 
            0, NULL, NULL
        );

        clFinish(context.opencl.queue);

        clSetKernelArg(
            context.opencl.transformation_kernel, 
            0, 
            sizeof(cl_mem), 
            &context.opencl.faces_mem_buffer
        );
    
        if (context.opencl.face_count * 2 > context.opencl.length_of_renderable_faces_array){
            debug(
                "old renderable faces count (%d) less than current (%d). Reallocating...", 
                RI_DEBUG_MESH_LOADER_REALLOCATION,
                context.opencl.length_of_renderable_faces_array, 
                context.opencl.face_count * 2
            );

            context.opencl.length_of_renderable_faces_array = context.opencl.face_count * 2;
            
            debug(
                "reallocating %f mb (%d renderable faces)",
                RI_DEBUG_MESH_LOADER_REALLOCATION, 
                sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array / 1048576.0, 
                context.opencl.length_of_renderable_faces_array
            );

            context.opencl.faces_to_render = RI_malloc(
                sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array
            );

            clerror = clReleaseMemObject(context.opencl.renderable_faces_mem_buffer);

            if (clerror != CL_SUCCESS){
                debug("couldn't free renderable faces memory buffer (error %d)", 
                    RI_DEBUG_MESH_LOADER_ERROR,
                    clerror);
                
                exit(1);
            }

            context.opencl.renderable_faces_mem_buffer = clCreateBuffer(
                context.opencl.context, 
                CL_MEM_READ_WRITE, 
                sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array, 
                NULL, &clerror
            );
    
            if (clerror != CL_SUCCESS){
                debug("couldn't reallocate renderable faces memory buffer (error %d)", 
                    RI_DEBUG_MESH_LOADER_ERROR,
                    clerror);

                exit(1);
            }
        }
    }

    fclose(file);
    
    end_time = clock();

    debug("loading mesh took %lf seconds", 
        RI_DEBUG_MESH_LOADER_TIME,
        (double)(end_time - start_time) / CLOCKS_PER_SEC);

    return new_mesh;
}

void RI_render(RI_scene *scene){
    clock_t start_time, end_time;
    
    start_time = clock();
  
    int local_group_size_x = 16;
    int local_group_size_y = 16;

    debug("---FRAME START-------------------------------------------\n", 
        RI_DEBUG_FRAME_START_END_MARKERS
    );

    int local_c_size_x = (int)fmin(context.opencl.num_h_tiles, local_group_size_x);
    int local_c_size_y = (int)fmin(context.opencl.num_v_tiles, local_group_size_y);

    const size_t c_global_work_size[2] = {
            local_c_size_x * ceil(context.opencl.num_h_tiles / (float)local_c_size_x), 
            local_c_size_y * ceil(context.opencl.num_v_tiles / (float)local_c_size_y)
        };

    const size_t c_local_work_size[2] = {
        (int)fmin(context.opencl.num_h_tiles, local_group_size_x), 
        (int)fmin(context.opencl.num_v_tiles, local_group_size_y)
    };

    clerror = clEnqueueNDRangeKernel(
        context.opencl.queue, 
        context.opencl.tile_clear_kernel, 
        2, 
        NULL, 
        c_global_work_size, 
        c_local_work_size, 
        0, NULL, &clevent
    );

    if (clerror != CL_SUCCESS) 
        debug("error enqueing tile clear kernel (%d)", 
            RI_DEBUG_TRANSFORMER_ERROR, 
            clerror
        );
    
    clFinish(context.opencl.queue);


    // transformer 
    
    double horizontal_fov_factor = (double)context.window.width / tanf(0.5 * scene->camera.FOV);
    double vertical_fov_factor = (double)context.window.height / tanf(0.5 * scene->camera.FOV);

    if (context.window.aspect_mode == RI_ASPECT_MODE_LETTERBOX) horizontal_fov_factor /= horizontal_fov_factor / vertical_fov_factor;

    // kernel args    
    
    // 18, double horizontal_fov_factor
    clSetKernelArg(context.opencl.transformation_kernel, 18, sizeof(double), &horizontal_fov_factor);
    // 19, double vertical_fov_factor
    clSetKernelArg(context.opencl.transformation_kernel, 19, sizeof(double), &vertical_fov_factor);

    // 20, double min_clip
    clSetKernelArg(context.opencl.transformation_kernel, 20, sizeof(float), &scene->camera.min_clip);
    // 21, double max_clip
    clSetKernelArg(context.opencl.transformation_kernel, 21, sizeof(float), &scene->camera.max_clip);

    // 22, double camera_x
    clSetKernelArg(context.opencl.transformation_kernel, 22, sizeof(double), &scene->camera.position.x);
    // 23, double camera_y
    clSetKernelArg(context.opencl.transformation_kernel, 23, sizeof(double), &scene->camera.position.y);
    // 24, double camera_z
    clSetKernelArg(context.opencl.transformation_kernel, 24, sizeof(double), &scene->camera.position.z);

    // 25, double camera_r_w
    clSetKernelArg(context.opencl.transformation_kernel, 25, sizeof(double), &scene->camera.rotation.w);
    // 26, double camera_r_x
    clSetKernelArg(context.opencl.transformation_kernel, 26, sizeof(double), &scene->camera.rotation.x);
    // 27, double camera_r_y
    clSetKernelArg(context.opencl.transformation_kernel, 27, sizeof(double), &scene->camera.rotation.y);
    // 28, double camera_r_z
    clSetKernelArg(context.opencl.transformation_kernel, 28, sizeof(double), &scene->camera.rotation.z);
    

    // count faces
    scene->face_count = 0;
    for (int actor_index = 0; actor_index < scene->length_of_actors_array; ++actor_index){
        scene->face_count += scene->actors[actor_index]->mesh->face_count;
    }


    // allocate faces_to_render if face count increases
    if (scene->face_count * 2 > context.opencl.length_of_renderable_faces_array){
        context.opencl.faces_to_render = RI_realloc(
            context.opencl.faces_to_render, 
            sizeof(RI_renderable_face) * scene->face_count * 2
        ); // x2 because faces can be split
    
        context.opencl.length_of_renderable_faces_array = scene->face_count * 2;

        debug(
            "old renderable faces count (%d) less than current (%d). Reallocating %f mb", 
            RI_DEBUG_RENDER_REALLOCATION,
            context.opencl.length_of_renderable_faces_array, scene->face_count * 2, 
            sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array / 1048576.0
        );

        context.opencl.faces_to_render = RI_malloc(
            sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array
        );

        clerror = clReleaseMemObject(context.opencl.renderable_faces_mem_buffer);

        if (clerror != CL_SUCCESS){
            debug("couldn't free renderable faces memory buffer (error %d)", 
                RI_DEBUG_RENDER_ERROR,
                clerror);
            
            exit(1);
        }

        context.opencl.renderable_faces_mem_buffer = clCreateBuffer(
            context.opencl.context, 
            CL_MEM_READ_WRITE, 
            sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array, 
            NULL, &clerror
        );

        if (clerror != CL_SUCCESS){
            debug("couldn't reallocate renderable faces memory buffer (error %d)", 
                RI_DEBUG_RENDER_ERROR,
                clerror);

            exit(1);
        }
    }
    
    // set faces_to_render to zero
    memset(context.opencl.faces_to_render, 0, sizeof(RI_renderable_face) * scene->face_count * 2);

    clSetKernelArg(
        context.opencl.transformation_kernel, 
        1, 
        sizeof(cl_mem), 
        &context.opencl.renderable_faces_mem_buffer
    );

    context.current_renderable_face_index = 0;
    context.current_split_renderable_face_index = 0;

    debug("transforming polygons...", RI_DEBUG_TRANSFORMER_MESSAGE);

    int renderable_face_index = 0;

    cl_ulong start, end;

    // transform polygons
    for (int actor_index = 0; actor_index < scene->length_of_actors_array; ++actor_index){
        RI_actor *actor = scene->actors[actor_index];
        
        if (scene->actors[actor_index]->mesh->face_count <= 0 || !scene->actors[actor_index]->active) continue;
        
        debug("actor index: %d face count: %d", 
            RI_DEBUG_TRANSFORMER_CURRENT_ACTOR, 
            actor_index, 
            actor->mesh->face_count
        );

        int face_sqrt = ceil(sqrt(scene->actors[actor_index]->mesh->face_count));

        int local_t_size = (int)fmin(face_sqrt, local_group_size_x);

        const size_t t_global_work_size[2] = {
            local_t_size * ceil(face_sqrt / (float)local_t_size), 
            local_t_size * ceil(face_sqrt / (float)local_t_size)
        };
        const size_t t_local_work_size[2] = {
            (int)fmin(face_sqrt, local_group_size_x), 
            (int)fmin(face_sqrt, local_group_size_y)
        };

        debug("transformer global work size: {%d, %d}", 
            RI_DEBUG_TRANSFORMER_GLOBAL_SIZE,
            t_global_work_size[0], 
            t_global_work_size[1]
        );

        debug("transformer local work size: {%d, %d}", 
            RI_DEBUG_TRANSFORMER_LOCAL_SIZE,
            t_local_work_size[0], 
            t_local_work_size[1]
        );

        debug(
            "(%d extra work items; %d items (%dx%d) - %d faces)", 
            RI_DEBUG_TRANSFORMER_EXTRA_WORK_ITEMS,
            t_global_work_size[0] * t_global_work_size[1] - scene->actors[actor_index]->mesh->face_count, 
            t_global_work_size[0] * t_global_work_size[1], 
            t_global_work_size[0], 
            t_global_work_size[1], 
            scene->actors[actor_index]->mesh->face_count
        );

        // 2, double actor_x
        clSetKernelArg(context.opencl.transformation_kernel, 2, sizeof(double), &actor->position.x);
        // 3, double actor_y
        clSetKernelArg(context.opencl.transformation_kernel, 3, sizeof(double), &actor->position.y);
        // 4, double actor_z
        clSetKernelArg(context.opencl.transformation_kernel, 4, sizeof(double), &actor->position.z);

        // 5, double actor_r_w
        clSetKernelArg(context.opencl.transformation_kernel, 5, sizeof(double), &actor->rotation.w);
        // 6, double actor_r_x
        clSetKernelArg(context.opencl.transformation_kernel, 6, sizeof(double), &actor->rotation.x);
        // 7, double actor_r_y
        clSetKernelArg(context.opencl.transformation_kernel, 7, sizeof(double), &actor->rotation.y);
        // 8, double actor_r_z
        clSetKernelArg(context.opencl.transformation_kernel, 8, sizeof(double), &actor->rotation.z);

        // 9, double actor_s_x
        clSetKernelArg(context.opencl.transformation_kernel, 9, sizeof(double), &actor->scale.x);
        // 10, double actor_s_y
        clSetKernelArg(context.opencl.transformation_kernel, 10, sizeof(double), &actor->scale.y);
        // 11, double actor_s_z
        clSetKernelArg(context.opencl.transformation_kernel, 11, sizeof(double), &actor->scale.z);

        // 12, int has_normals
        clSetKernelArg(context.opencl.transformation_kernel, 12, sizeof(int), &actor->mesh->has_normals);
        // 13, int has_uvs
        clSetKernelArg(context.opencl.transformation_kernel, 13, sizeof(int), &actor->mesh->has_uvs);
        // 14, int face_array_offset_index
        clSetKernelArg(context.opencl.transformation_kernel, 14, sizeof(int), &actor->mesh->face_index);
        // 15, int face_count
        clSetKernelArg(context.opencl.transformation_kernel, 15, sizeof(int), &actor->mesh->face_count);

        // 29, int renderable_face_offset
        clSetKernelArg(context.opencl.transformation_kernel, 29, sizeof(int), &renderable_face_index);
        
        // 30, int face_sqrt
        clSetKernelArg(context.opencl.transformation_kernel, 30, sizeof(int), &face_sqrt);

        debug("texture width: %d texture height: %d texture index %d", RI_DEBUG_TRANSFORMER_TEXTURE, actor->texture->width, actor->texture->height, actor->texture->index);

        // 31: uint16_t texture_width
        clSetKernelArg(context.opencl.transformation_kernel, 31, sizeof(uint16_t), &actor->texture->width);
        // 32: uint16_t texture_height
        clSetKernelArg(context.opencl.transformation_kernel, 32, sizeof(uint16_t), &actor->texture->height);
        // 33: uint32_t texture_index
        clSetKernelArg(context.opencl.transformation_kernel, 33, sizeof(uint32_t), &actor->texture->index);

        debug("running actor #%d's transformation kernel...", 
            RI_DEBUG_TRANSFORMER_MESSAGE, 
            actor_index
        );

        clerror = clEnqueueNDRangeKernel(
            context.opencl.queue, 
            context.opencl.transformation_kernel, 
            2, 
            NULL, 
            t_global_work_size, 
            t_local_work_size, 
            0, NULL, &clevent
        );

        if (clerror != CL_SUCCESS) 
            debug("error enqueing transformation kernel (%d)", 
                RI_DEBUG_TRANSFORMER_ERROR, 
                clerror
            );
        
        clFinish(context.opencl.queue);

        clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
        clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);

        double ns = (double)(end - start);
        debug("actor #%d's transformation kernel took %f seconds\n", 
            RI_DEBUG_TRANSFORMER_TIME, 
            actor_index, 
            ns / 1e9
        );
    
        renderable_face_index += actor->mesh->face_count * 2;
    }

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

    debug("rasterizer global work size: {%d, %d}", 
        RI_DEBUG_RASTERIZER_GLOBAL_SIZE,
        x, 
        y
    );

    debug("rasterizer local work size: {%d, %d}", 
        RI_DEBUG_RASTERIZER_GLOBAL_SIZE,
        local_group_size_x, 
        local_group_size_y
    );    

    // kernel args
    clSetKernelArg(
        context.opencl.rasterization_kernel, 
        0, 
        sizeof(cl_mem), 
        &context.opencl.renderable_faces_mem_buffer
    );
    clSetKernelArg(context.opencl.rasterization_kernel, 7, sizeof(int), &scene->face_count);
    clSetKernelArg(
        context.opencl.rasterization_kernel, 
        8, 
        sizeof(int), 
        &context.current_split_renderable_face_index
    );

    debug("rasterizing...", RI_DEBUG_RASTERIZER_MESSAGE);

    // run raster kernel
    clEnqueueNDRangeKernel(
        context.opencl.queue, 
        context.opencl.rasterization_kernel, 
        2, 
        NULL, 
        r_global_work_size, 
        r_local_work_size, 
        0, NULL, &clevent
    );
    clFinish(context.opencl.queue);

    clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
    clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);

    debug("rasterization kernel took %f seconds\n", 
        RI_DEBUG_RASTERIZER_TIME, 
        (double)(end - start) / 1e9
    );

    // put GPU frame buffer into CPU
    clEnqueueReadBuffer(
        context.opencl.queue, 
        context.opencl.frame_buffer_mem_buffer, 
        CL_TRUE, 
        0, 
        context.window.width * context.window.height * sizeof(uint32_t), 
        context.sdl.frame_buffer, 
        0, NULL, &clevent
    );
    clFinish(context.opencl.queue);

    clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
    clGetEventProfilingInfo(clevent, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);

    debug("reading GPU frame buffer took %f seconds", 
        RI_DEBUG_RENDER_FRAME_BUFFER_READ_TIME,
        (double)(end - start) / 1e9
    );

    context.opencl.number_of_faces_just_rendered += scene->face_count;

    end_time = clock();

    debug("rendering took %lf seconds", 
        RI_DEBUG_RENDER_TIME, 
        (double)(end_time - start_time) / CLOCKS_PER_SEC
    );
}

void RI_tick(){
    clock_t start_time, end_time;
    
    start_time = clock();
    
    SDL_Event event;

    while (SDL_PollEvent(&event)){
        context.sdl.event_handler(event);
    }

    SDL_LockTexture(
        context.sdl.frame_buffer_texture, 
        NULL, 
        (void*)&context.sdl.frame_buffer, 
        &context.sdl.pitch
    );

    SDL_UnlockTexture(context.sdl.frame_buffer_texture);

    SDL_RenderCopy(context.sdl.renderer, context.sdl.frame_buffer_texture, NULL, NULL);
    SDL_RenderPresent(context.sdl.renderer);

    ++context.current_frame;

    end_time = clock();

    debug("Done! ticking took %lf seconds", 
        RI_DEBUG_TICK_TIME,
        (double)(end_time - start_time) / CLOCKS_PER_SEC
    );

    debug("---FRAME END---(frame #%d, %d polygons)----------------\n", 
        RI_DEBUG_FRAME_START_END_MARKERS,
        context.current_frame, 
        context.opencl.number_of_faces_just_rendered
    );

    context.opencl.number_of_faces_just_rendered = 0;

    return;
}

void RI_default_SDL_event_handler(SDL_Event event){
    switch (event.type){
        case SDL_QUIT: {
            context.is_running = ri_false;

            break;
        }

        default: {
            break;
        }
    }

    return;
}

RI_context *RI_get_context(){
    context.sdl = (RI_SDL){NULL, NULL, NULL, NULL, &RI_default_SDL_event_handler, -1};
    context.opencl = (RI_CL){NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 16, 16, 0, 0, 0};
    context.window = (RI_window){800, 800, 400, 400, "RasterIver Window", RI_ASPECT_MODE_LETTERBOX};
    
    context.debug_flags = RI_DEBUG_ERRORS;
    context.current_frame = 0;
    context.is_running = ri_true;
    context.debug_prefix = "[RasterIver] ";

    return &context;
} 

// Convert a CL file to a string
char *load_kernel_source(const char *filename) {
    FILE *f = fopen(filename, "rb");

    if (f == NULL){
        debug("couldn't open kernel file \"%s\"", 
            RI_DEBUG_KERNEL_LOADER_ERROR, 
            filename
        );
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
    
    // init SDL
    context.sdl.window = SDL_CreateWindow(
        context.window.title, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        context.window.width, 
        context.window.height, 0
    );
    context.sdl.renderer = SDL_CreateRenderer(context.sdl.window, -1, SDL_RENDERER_ACCELERATED);
    context.sdl.frame_buffer_texture = SDL_CreateTexture(
        context.sdl.renderer, 
        SDL_PIXELFORMAT_BGRA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        context.window.width, 
        context.window.height
    );

    context.sdl.frame_buffer = malloc(
        sizeof(uint32_t) * context.window.width * context.window.height
    );
    
    if (!context.debug_flags)
        context.debug_flags = RI_DEBUG_ERRORS;
        

    // init OpenCL
        
    context.opencl.faces_to_render = RI_malloc(
        sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array
    );
    context.opencl.faces = RI_malloc(sizeof(RI_face) * context.opencl.face_count);

    cl_uint num_platforms = 0;
    clerror = clGetPlatformIDs(0, NULL, &num_platforms);

    cl_platform_id *platforms = malloc(sizeof(cl_platform_id) * num_platforms);
    clerror = clGetPlatformIDs(num_platforms, platforms, NULL);

    cl_device_id *devices; 
    
    cl_platform_id chosen_platform = NULL;
    char pname[256];
    for (cl_uint i = 0; i < num_platforms; i++) {
        clerror = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(pname), pname, NULL);

        debug("get platform info result: %d", RI_DEBUG_INIT_PLATFORMS, clerror);

        cl_uint num_devices = 0;
        clerror = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

        debug("num devices result: %d", RI_DEBUG_INIT_PLATFORMS, clerror);

        devices = malloc(sizeof(cl_device_id) * num_devices);
        clerror = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

        if (i == 0){
            context.opencl.device = devices[0];
        }

        debug("get device ids result: %d", RI_DEBUG_INIT_PLATFORMS, clerror);

        debug("-platform #%d: NAME: %s | DEVICE COUNT: %d", RI_DEBUG_INIT_PLATFORMS, i, pname, num_devices);

        for (cl_uint j = 0; j < num_devices; ++j){
            debug("-\\ device #%d: ID: %u", RI_DEBUG_INIT_PLATFORMS, j, devices[j]);
        }

        free(devices);
    }

    context.opencl.platform = platforms[0];

    debug("chosen device id: %u", RI_DEBUG_INIT_PLATFORMS, context.opencl.device);

    context.opencl.context = clCreateContext(NULL, 1, &context.opencl.device, NULL, NULL, NULL);
    context.opencl.queue = clCreateCommandQueueWithProperties(
        context.opencl.context, 
        context.opencl.device, 
        (const cl_queue_properties[]){CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0}, 
        &clerror
    );

    if (!context.opencl.context){
        debug("failed to create OpenCL context", RI_DEBUG_OPENCL_ERROR);
    
        exit(1);
    }

    // build programs

    char *program_source = load_kernel_source("src/kernels/kernels.cl");    
    cl_program rasterization_program = clCreateProgramWithSource(
        context.opencl.context, 
        1, 
        (const char**)&program_source, 
        NULL, NULL
    );
    
    free(program_source);

    cl_int result = clBuildProgram(rasterization_program, 1, &context.opencl.device, "", NULL, NULL);

    if (result != CL_SUCCESS){
        char log[10001];

        clGetProgramBuildInfo(
            rasterization_program, 
            context.opencl.device, 
            CL_PROGRAM_BUILD_LOG, 
            10000, log, NULL);

        debug("rasterization program build failed (%d). Log: \n  %s", RI_DEBUG_OPENCL_ERROR, result, log);
    
        return 1;
    }

    // kernels

    context.opencl.rasterization_kernel = clCreateKernel(rasterization_program, "rasterizer", &clerror);

    if (clerror != CL_SUCCESS){
        debug("couldn't create rasterizer kernel", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }

    context.opencl.transformation_kernel = clCreateKernel(rasterization_program, "transformer", &clerror);

    if (clerror != CL_SUCCESS){
        debug("couldn't create transformer kernel", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }

    context.opencl.tile_clear_kernel = clCreateKernel(rasterization_program, "clear_tile_array", &clerror);

    if (clerror != CL_SUCCESS){
        debug("couldn't create tile array clearer kernel", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }

    // rasterizer

    context.opencl.renderable_faces_mem_buffer = clCreateBuffer(
        context.opencl.context, 
        CL_MEM_READ_WRITE, 
        sizeof(RI_renderable_face) * context.opencl.length_of_renderable_faces_array, 
        NULL, &clerror);
    
    if (clerror != CL_SUCCESS){
        debug("couldn't create renderable faces memory buffer", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }
    
    context.opencl.frame_buffer_mem_buffer = clCreateBuffer(
        context.opencl.context, 
        CL_MEM_READ_WRITE, 
        sizeof(uint32_t) * context.window.width * context.window.height, 
        NULL, &clerror);

    if (clerror != CL_SUCCESS || !context.opencl.frame_buffer_mem_buffer){
        debug("couldn't create frame buffer memory buffer", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }

    context.opencl.textures_mem_buffer = clCreateBuffer(
        context.opencl.context, 
        CL_MEM_READ_WRITE, 
        1, 
        NULL, &clerror);

    if (clerror != CL_SUCCESS || !context.opencl.textures_mem_buffer){
        debug("couldn't create textures memory buffer", RI_DEBUG_OPENCL_ERROR);
        return 1;
    }

// rasterizer(__global RI_renderable_face *renderable_faces, __global uint *frame_buffer, 
//            int width, int height, int half_width, int half_height, int number_of_renderable_faces, 
//            int number_of_split_renderable_faces)

    clSetKernelArg(context.opencl.rasterization_kernel, 0, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 1, sizeof(cl_mem), &context.opencl.textures_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 2, sizeof(cl_mem), &context.opencl.frame_buffer_mem_buffer);
    clSetKernelArg(context.opencl.rasterization_kernel, 3, sizeof(int), &context.window.width);
    clSetKernelArg(context.opencl.rasterization_kernel, 4, sizeof(int), &context.window.height);
    clSetKernelArg(context.opencl.rasterization_kernel, 5, sizeof(int), &context.window.half_width);
    clSetKernelArg(context.opencl.rasterization_kernel, 6, sizeof(int), &context.window.half_height);
    clSetKernelArg(context.opencl.rasterization_kernel, 7, sizeof(int), &context.current_renderable_face_index);
    clSetKernelArg(context.opencl.rasterization_kernel, 8, sizeof(int), &context.current_split_renderable_face_index);
    clSetKernelArg(context.opencl.rasterization_kernel, 9, sizeof(cl_mem), &context.opencl.tiles_mem_buffer);

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

    // // 0: __global RI_face *faces
    // clSetKernelArg(context.opencl.transformation_kernel, 0, sizeof(cl_mem), &context.opencl.faces_mem_buffer);

    // // 1: __global RI_renderable_face *renderable_faces
    // clSetKernelArg(context.opencl.transformation_kernel, 1, sizeof(cl_mem), &context.opencl.renderable_faces_mem_buffer);

    // // 2: double actor_x
    // clSetKernelArg(context.opencl.transformation_kernel, 2, sizeof(double), &actor_x);
    // // 3: double actor_y
    // clSetKernelArg(context.opencl.transformation_kernel, 3, sizeof(double), &actor_y);
    // // 4: double actor_z
    // clSetKernelArg(context.opencl.transformation_kernel, 4, sizeof(double), &actor_z);

    // // 5: double actor_r_w
    // clSetKernelArg(context.opencl.transformation_kernel, 5, sizeof(double), &actor_r_w);
    // // 6: double actor_r_x
    // clSetKernelArg(context.opencl.transformation_kernel, 6, sizeof(double), &actor_r_x);
    // // 7: double actor_r_y
    // clSetKernelArg(context.opencl.transformation_kernel, 7, sizeof(double), &actor_r_y);
    // // 8: double actor_r_z
    // clSetKernelArg(context.opencl.transformation_kernel, 8, sizeof(double), &actor_r_z);

    // // 9: double actor_s_x
    // clSetKernelArg(context.opencl.transformation_kernel, 9, sizeof(double), &actor_s_x);
    // // 10: double actor_s_y
    // clSetKernelArg(context.opencl.transformation_kernel, 10, sizeof(double), &actor_s_y);
    // // 11: double actor_s_z
    // clSetKernelArg(context.opencl.transformation_kernel, 11, sizeof(double), &actor_s_z);

    // // 12: int has_normals
    // clSetKernelArg(context.opencl.transformation_kernel, 12, sizeof(int), &has_normals);
    // // 13: int has_uvs
    // clSetKernelArg(context.opencl.transformation_kernel, 13, sizeof(int), &has_uvs);

    // // 14: int face_array_offset_index
    // clSetKernelArg(context.opencl.transformation_kernel, 14, sizeof(int), &face_array_offset_index);
    // // 15: int face_count
    // clSetKernelArg(context.opencl.transformation_kernel, 15, sizeof(int), &face_count);

    // // 16: int width
    clSetKernelArg(context.opencl.transformation_kernel, 16, sizeof(int), &context.window.width);
    // // 17: int height
    clSetKernelArg(context.opencl.transformation_kernel, 17, sizeof(int), &context.window.height);

    // // 18: double horizontal_fov_factor
    // clSetKernelArg(context.opencl.transformation_kernel, 18, sizeof(double), &horizontal_fov_factor);
    // // 19: double vertical_fov_factor
    // clSetKernelArg(context.opencl.transformation_kernel, 19, sizeof(double), &vertical_fov_factor);

    // // 20: float min_clip
    // clSetKernelArg(context.opencl.transformation_kernel, 20, sizeof(float), &min_clip_f);
    // // 21: float max_clip
    // clSetKernelArg(context.opencl.transformation_kernel, 21, sizeof(float), &max_clip_f);

    // // 22: double camera_x
    // clSetKernelArg(context.opencl.transformation_kernel, 22, sizeof(double), &camera_x);
    // // 23: double camera_y
    // clSetKernelArg(context.opencl.transformation_kernel, 23, sizeof(double), &camera_y);
    // // 24: double camera_z
    // clSetKernelArg(context.opencl.transformation_kernel, 24, sizeof(double), &camera_z);

    // // 25: double camera_r_w
    // clSetKernelArg(context.opencl.transformation_kernel, 25, sizeof(double), &camera_r_w);
    // // 26: double camera_r_x
    // clSetKernelArg(context.opencl.transformation_kernel, 26, sizeof(double), &camera_r_x);
    // // 27: double camera_r_y
    // clSetKernelArg(context.opencl.transformation_kernel, 27, sizeof(double), &camera_r_y);
    // // 28: double camera_r_z
    // clSetKernelArg(context.opencl.transformation_kernel, 28, sizeof(double), &camera_r_z);

    // // 29: int renderable_face_offset
    // clSetKernelArg(context.opencl.transformation_kernel, 29, sizeof(int), &renderable_face_offset);
    // // 30: int face_sqrt
    // clSetKernelArg(context.opencl.transformation_kernel, 30, sizeof(int), &face_sqrt);
    // // 31: uint16_t texture_width
    // clSetKernelArg(context.opencl.transformation_kernel, 31, sizeof(uint16_t), &texture_width);
    // // 32: uint16_t texture_height
    // clSetKernelArg(context.opencl.transformation_kernel, 32, sizeof(uint16_t), &texture_height);
    // // 33: uint32_t texture_index
    // clSetKernelArg(context.opencl.transformation_kernel, 33, sizeof(uint32_t), &texture_index);
    
    // // 34: uint32_t tiles mem buffer
    // clSetKernelArg(context.opencl.transformation_kernel, 34, sizeof(cl_mem), &context.opencl.tiles_mem_buffer);
    
    context.defaults.default_actor = RI_malloc(sizeof(RI_actor));

    context.defaults.default_actor->mesh = RI_load_mesh("objects/error_object.obj");
    context.defaults.default_actor->active = 1;
    context.defaults.default_actor->material_index = 0;
    context.defaults.default_actor->position = (RI_vector_3){0, 0, 0};
    context.defaults.default_actor->rotation = (RI_vector_4){1, 0, 0, 0};
    context.defaults.default_actor->scale = (RI_vector_3){1, 1, 1};
    context.defaults.default_actor->texture = RI_load_image("textures/missing_texture.bmp");                                                                                                                                                   

    return 0;
}