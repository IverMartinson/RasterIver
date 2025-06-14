#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>
#include "../headers/rasteriver.h"
#include <stdarg.h>
#include <stdio.h>
#include <SDL2/SDL_ttf.h>

// ----- Internal Variables
int ri_width;
int ri_height;

int selected_triangle = 0;

int ri_h_width;
int ri_h_height;

float highest_z = 0;

int polygon_count;
RI_polygons polygons = NULL;

int object_count;
RI_objects objects;
RI_verticies verticies;
RI_verticies split_verticies;
RI_verticies transformed_verticies;
RI_verticies normals;
RI_verticies transformed_normals;
RI_verticies uvs;
RI_triangles triangles;
int *split_triangles;
RI_textures textures;
int textures_size;
int texture_count;

int running = 1;
int frame = 0;

int show_buffer = RI_BUFFER_COMPLETE;
int show_debug = 0;
int debug_level = RI_DEBUG_LOW;
int show_fps = 0;
int debug_fps = 0;
int clean_polygons = 0;
int populate_polygons = 0;
int be_master_renderer = 0;
int debug_frame = 0;
int show_frame = 0;
int show_info = 0;
int debug_tick = 0;
int use_cpu = 0;
int handle_events = 1;

Uint64 start_time;
double frame_time_ms;
double fps;
double elapsed_ticks;
double delta_time;
int fps_cap = -1;

char prefix[50] = "[RasterIver] ";
// ----- Internal Variables

// ----- Rendering Vars
SDL_Event event;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

int *texture_info;

float near_clip = 0.01;
float wireframe_width = 0.05;

float fov = 90 * RI_PI / 180;

RI_uint *frame_buffer;

char *font_file = "src/fonts/OxygenMono.ttf";
int font_size = 24;
TTF_Font *font;
SDL_Color font_color = {255, 255, 255, 255};

SDL_Surface *text_surface;
SDL_Texture *text_texture;
SDL_Rect text_rect;
// ----- Rendering Vars

// ----- OpenCL Vars
cl_platform_id platform;
cl_device_id device;
RI_uint number_of_platforms, number_of_devices;

cl_int error;
cl_context context;
cl_command_queue queue;

cl_mem input_memory_buffer;
cl_mem output_memory_buffer;

cl_mem object_memory_buffer;
cl_mem triangles_memory_buffer;
cl_mem verticies_memory_buffer;
cl_mem transformed_verticies_memory_buffer;
cl_mem normals_memory_buffer;
cl_mem transformed_normals_memory_buffer;
cl_mem uvs_memory_buffer;
cl_mem textures_memory_buffer;
cl_mem texture_info_memory_buffer;

cl_program kernel_program_non_master;
cl_kernel compiled_kernel_non_master;

cl_program kernel_program_master;
cl_kernel compiled_kernel_master;

cl_program kernel_program_transformer;
cl_kernel compiled_kernel_transformer;

size_t size_2d[2];
size_t local_size;

RI_uint pattern;
// ----- OpenCL Vars

// ----- Internal Functions
RI_result debug(int verbose, char *string, ...){
    if (!show_debug || verbose > debug_level){
        return RI_ERROR;
    }

    va_list args;
    va_start(args, string);

    char message[500];

    strcpy(message, prefix);

    strcat(message, string);

    vprintf(message, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

RI_result debug_tick_func(int verbose, char *string, ...)
{
    if (!show_debug || (verbose > debug_level) || !debug_tick){
        return RI_ERROR;
    }

    va_list args;
    va_start(args, string);

    char message[500];

    strcpy(message, prefix);

    strcat(message, string);

    vprintf(message, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

const char* code_to_string(cl_int code) {
    switch (code) {case -1:  return "CL_DEVICE_NOT_FOUND";case -2:  return "CL_DEVICE_NOT_AVAILABLE";case -3:  return "CL_COMPILER_NOT_AVAILABLE";case -4:  return "CL_MEM_OBJECT_ALLOCATION_FAILURE";case -5:  return "CL_OUT_OF_RESOURCES";case -6:  return "CL_OUT_OF_HOST_MEMORY";case -7:  return "CL_PROFILING_INFO_NOT_AVAILABLE";case -8:  return "CL_MEM_COPY_OVERLAP";case -9:  return "CL_IMAGE_FORMAT_MISMATCH";case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";case -11: return "CL_BUILD_PROGRAM_FAILURE";case -12: return "CL_MAP_FAILURE";case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";case -15: return "CL_COMPILE_PROGRAM_FAILURE";case -16: return "CL_LINKER_NOT_AVAILABLE";case -17: return "CL_LINK_PROGRAM_FAILURE";case -18: return "CL_DEVICE_PARTITION_FAILED";case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";case -30: return "CL_INVALID_VALUE";case -31: return "CL_INVALID_DEVICE_TYPE";case -32: return "CL_INVALID_PLATFORM";case -33: return "CL_INVALID_DEVICE";case -34: return "CL_INVALID_CONTEXT";case -35: return "CL_INVALID_QUEUE_PROPERTIES";case -36: return "CL_INVALID_COMMAND_QUEUE";case -37: return "CL_INVALID_HOST_PTR";case -38: return "CL_INVALID_MEM_OBJECT";case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";case -40: return "CL_INVALID_IMAGE_SIZE";case -41: return "CL_INVALID_SAMPLER";case -42: return "CL_INVALID_BINARY";case -43: return "CL_INVALID_BUILD_OPTIONS";case -44: return "CL_INVALID_PROGRAM";case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";case -46: return "CL_INVALID_KERNEL_NAME";case -47: return "CL_INVALID_KERNEL_DEFINITION";case -48: return "CL_INVALID_KERNEL";case -49: return "CL_INVALID_ARG_INDEX";case -50: return "CL_INVALID_ARG_VALUE";case -51: return "CL_INVALID_ARG_SIZE";case -52: return "CL_INVALID_KERNEL_ARGS";case -53: return "CL_INVALID_WORK_DIMENSION";case -54: return "CL_INVALID_WORK_GROUP_SIZE";case -55: return "CL_INVALID_WORK_ITEM_SIZE";case -56: return "CL_INVALID_GLOBAL_OFFSET";case -57: return "CL_INVALID_EVENT_WAIT_LIST";case -58: return "CL_INVALID_EVENT";case -59: return "CL_INVALID_OPERATION";case -60: return "CL_INVALID_GL_OBJECT";case -61: return "CL_INVALID_BUFFER_SIZE";case -62: return "CL_INVALID_MIP_LEVEL";case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";case -64: return "CL_INVALID_PROPERTY";case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";case -66: return "CL_INVALID_COMPILER_OPTIONS";case -67: return "CL_INVALID_LINKER_OPTIONS";case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";default:  return "Unknown OpenCL error code";}
}

RI_result erchk_func(cl_int error, int line, char *file)
{
    if (error != CL_SUCCESS)
    {
        debug(RI_DEBUG_LOW, "OpenCL Error: %d (%s) at line %d at file %s", error, code_to_string(error), line, file);

        RI_Stop(0);
    }

    return RI_SUCCESS;
}

#define erchk(error) erchk_func(error, __LINE__, __FILE__)
// ----- Internal Functions

// ----- Value Return Functions
RI_result RI_IsRunning()
{
    if (running)
    {
        return RI_RUNNING;
    }
    else
    {
        return RI_NOT_RUNNING;
    }
}

SDL_Event RI_GetLastSDLEvent(){
    return event;
}

RI_result RI_ListFlags(){
    printf("RI_FLAG_DEBUG: Turns debugging on or off\n");
    printf("RI_FLAG_DEBUG_VERBOSE: If debugging and verbose is on, print extra data\n");
    printf("RI_FLAG_SHOW_Z_BUFFER: Whether or not to render the Z buffer");
    printf("RI_FLAG_SHOW_FPS: Render FPS on screen");
    printf("RI_FLAG_DEBUG_FPS: Debug FPS into the console");
    printf("RI_FLAG_CLEAN_POLYGONS: When requesting polygons, write INF to the array. (INF means a triangle doesn't exist. Useful for if you allocate more space than there are triangles, but inefficient if you call RI_RequestPolygons frequently\n)");
    printf("RI_FLAG_POPULATE_POLYGONS: When requesting polygons, populate the array with random triangles (useful for testing/benchmark)\n");
    printf("RI_FLAG_BE_MASTER_RENDERER: If this is 0, RasterIver only acts as a polygon renderer. If 1, you get all the features like objects and rotations.\n");

    return RI_SUCCESS;
}
// ----- Value Return Functions

// ----- Set Value Functions
RI_result RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value){
    debug(RI_DEBUG_HIGH, "Called RI_SetFlag");

    switch (RI_FlagToSet)
    {
    case RI_FLAG_DEBUG:
        show_debug = RI_Value;
        break;

    case RI_FLAG_DEBUG_LEVEL:
        debug_level = RI_Value;
        break;

    case RI_FLAG_SHOW_BUFFER:
        show_buffer = RI_Value;
        break;

    case RI_FLAG_SHOW_FPS:
        show_fps = RI_Value;
        break;

    case RI_FLAG_DEBUG_FPS:
        debug_fps = RI_Value;
        break;

    case RI_FLAG_CLEAN_POLYGONS:
        clean_polygons = RI_Value;
        break;
    
    case RI_FLAG_POPULATE_POLYGONS:
        populate_polygons = RI_Value;
        break;

    case RI_FLAG_BE_MASTER_RENDERER:
        be_master_renderer = RI_Value;
        break;

    case RI_FLAG_DEBUG_FRAME:
        debug_frame = RI_Value;
        break;

    case RI_FLAG_SHOW_FRAME:
        show_frame = RI_Value;
        break;

    case RI_FLAG_SHOW_INFO:
        show_info = RI_Value;
        break;

    case RI_FLAG_DEBUG_TICK:
        debug_tick = RI_Value;
        break;
        
    case RI_FLAG_USE_CPU:
        use_cpu = RI_Value;
        break;

    case RI_FLAG_HANDLE_SDL_EVENTS:
        handle_events = RI_Value;
        break;

    default:
        return RI_INVALID_FLAG;
    }

    return RI_SUCCESS;
}

RI_result RI_SetValue(RI_value RI_ValueToSet, float RI_Value){
    debug(RI_DEBUG_HIGH, "Called RI_SetValue");

    switch (RI_ValueToSet)
    {
    case RI_VALUE_WIREFRAME_SCALE:
        wireframe_width = RI_Value;
        break;

    case RI_VALUE_SELECTED_TRIANGLE:
        selected_triangle = RI_Value;
        break;

    case RI_VALUE_MINIMUM_CLIP:
        near_clip = RI_Value;

    default:
        return RI_INVALID_VALUE;
    }

    return RI_SUCCESS;
}

RI_result RI_SetBackground(RI_uint RI_BackgroundColor){
    debug(RI_DEBUG_HIGH, "Called RI_SetBackground");

    pattern = RI_BackgroundColor;

    return RI_SUCCESS;
}

RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest){
    debug(RI_DEBUG_HIGH, "Called RI_RequestPolygons");

    polygon_count = RI_PolygonsToRequest;
    
    int size = sizeof(float) * 3 * 3 * polygon_count;
    
    debug(RI_DEBUG_MEDIUM, "Requesting %d Polygons... (%d bytes)", polygon_count, size);
    
    if (polygons != NULL)
    {
        free(polygons);
    }
    
    polygons = malloc(size);
    
    if (polygons == NULL)
    {
        debug(RI_DEBUG_LOW, "Malloc Error");
        return (float*)RI_ERROR;
    }
    
    for (int i_polygon = 0; i_polygon < polygon_count * 9; i_polygon += 3){     
        if (clean_polygons){
            polygons[i_polygon] = INFINITY;
            polygons[i_polygon + 1] = INFINITY;
            polygons[i_polygon + 2] = INFINITY;
        }
        else if (populate_polygons){
            polygons[i_polygon] = rand() % ri_width;
            polygons[i_polygon + 1] = rand() % ri_height;
            polygons[i_polygon + 2] = rand() % ((ri_width + ri_height) / 2);
        }
    }
    
    input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, polygons, &error);
    
    if (input_memory_buffer == NULL)
    {
        debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Requested Polygons");
    }

    erchk(error);
    
    debug(RI_DEBUG_MEDIUM, "Request for %d Polygons Granted", polygon_count);
    
    return polygons;
}

int vertex_count = 0;
int normal_count = 0;
int uv_count = 0;
int face_count = 0;

int loading_object_current_verticies_count = 0;
int loading_object_current_normals_count = 0;
int loading_object_current_uvs_count = 0;
int loading_object_current_faces_count = 0;

void slice(char *string, char *result, int start, int end){
    debug(RI_DEBUG_HIGH, "Called slice");

    strncpy(result, string + start, end - start);
}

int malloc_objects(int objects, char **file_names, char **allocated_file_names, int *object_file_offsets){
    debug(RI_DEBUG_HIGH, "Called malloc_objects");

    int loaded_object_count = 0; 

    int malloc_face_count = 0;
    int malloc_vertex_count = 0;
    int malloc_normal_count = 0;
    int malloc_uv_count = 0;    

    int is_this_object_file_already_in_the_object_files_array = 0;

    for (int i = 0; i < objects; i++){
        is_this_object_file_already_in_the_object_files_array = 0;

        for (int i_object_file = 0; i_object_file < object_count + objects; i_object_file++){
            debug(RI_DEBUG_HIGH, "object %d, object file %d (%s  -> %s)", i, i_object_file, file_names[i], allocated_file_names[i_object_file]);

            if (strcmp(allocated_file_names[i_object_file], file_names[i]) == 0){
                debug(RI_DEBUG_HIGH, "Not Reloading Object \"%s\" (object #%d) (compared %s to %s)", file_names[i], i_object_file, allocated_file_names[i_object_file], file_names[i]);

                is_this_object_file_already_in_the_object_files_array = 1;

                object_file_offsets[i * 5 + 0] = object_file_offsets[i_object_file * 5 + 0];
                object_file_offsets[i * 5 + 1] = object_file_offsets[i_object_file * 5 + 1];
                object_file_offsets[i * 5 + 2] = object_file_offsets[i_object_file * 5 + 2];
                object_file_offsets[i * 5 + 3] = object_file_offsets[i_object_file * 5 + 3];
                object_file_offsets[i * 5 + 4] = -object_file_offsets[i_object_file * 5 + 4];

                break;
            }
        }    

        if (!is_this_object_file_already_in_the_object_files_array){
            object_file_offsets[i * 5 + 0] = malloc_face_count;
            object_file_offsets[i * 5 + 1] = malloc_vertex_count;
            object_file_offsets[i * 5 + 2] = malloc_normal_count;
            object_file_offsets[i * 5 + 3] = malloc_uv_count;
            
            allocated_file_names[loaded_object_count] = file_names[i];
            loaded_object_count++;

            debug(RI_DEBUG_HIGH, "file_names[i] is %s", file_names[i]);

            FILE *file = fopen(file_names[i], "r");

            if(file == NULL){
                debug(RI_DEBUG_LOW, "Error Opening Object File \"%s\"", file_names[i]);
                
                file = fopen("objects/error_object.obj", "r");
            }

            char line[256];

            int tri_count = 0;

            while (fgets(line, sizeof(line), file)) {
                if (line[0] == 'f' && line[1] == ' ') {
                    malloc_face_count++;
                    tri_count++;
                } 
                else if (line[0] == 'v' && line[1] == ' ') {
                    malloc_vertex_count++;
                } 
                else if (line[0] == 'v' && line[1] == 'n') {
                    malloc_normal_count++;
                } 
                else if (line[0] == 'v' && line[1] == 't') {
                    malloc_uv_count++;
                } 
            }

            object_file_offsets[i * 5 + 4] = i + 1;
            
            fclose(file);
        }
    }

    face_count = malloc_face_count;
    vertex_count = malloc_vertex_count;
    normal_count = malloc_normal_count;
    uv_count = malloc_uv_count;

    if(verticies != NULL){
        free(verticies);
    } 

    if(normals != NULL){
        free(normals);
    } 
    
    if(uvs != NULL){
        free(uvs);
    } 

    if(triangles != NULL){
        free(triangles);
    } 

    debug(RI_DEBUG_MEDIUM, "Allocated Vertecies: %d", vertex_count);
    debug(RI_DEBUG_MEDIUM, "Allocated Normals: %d", normal_count);
    debug(RI_DEBUG_MEDIUM, "Allocated UVs: %d", uv_count);
    debug(RI_DEBUG_MEDIUM, "Allocated Triangles: %d", face_count);

    if (vertex_count > 0){
        verticies = malloc(sizeof(RI_verticies) * vertex_count * vs);
    }
    
    if (normal_count > 0){
        normals = malloc(sizeof(RI_verticies) * normal_count * vs);
    }

    if (uv_count > 0){
        uvs = malloc(sizeof(RI_verticies) * uv_count * vs);
    }

    if (face_count > 0){
        triangles = malloc(sizeof(RI_triangles) * face_count * ts);
    }

    debug(RI_DEBUG_HIGH, "Left malloc_objects");

    return loaded_object_count;
}

load_object_return load_object(char *object_path, int object_offset, int base){
    debug(RI_DEBUG_HIGH, "Called load_object");

    debug(RI_DEBUG_MEDIUM, "Loading Object #%d...", object_offset + 1);

    debug(RI_DEBUG_MEDIUM, "Opening File \"%s\"...", object_path);

    FILE *file = fopen(object_path, "r");

    if(file == NULL){
        file = fopen("objects/error_object.obj", "r");
    }

    char line[256];
    
    // current values
    int ct = 0;
    int cn = 0;
    int cv = 0;
    int cu = 0;

    int obj_face_type = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') {

            int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d/", 
                &triangles[(ct + loading_object_current_faces_count) * ts + 0], &triangles[(ct + loading_object_current_faces_count) * ts + 6], &triangles[(ct + loading_object_current_faces_count) * ts + 3], 
                &triangles[(ct + loading_object_current_faces_count) * ts + 1], &triangles[(ct + loading_object_current_faces_count) * ts + 7], &triangles[(ct + loading_object_current_faces_count) * ts + 4], 
                &triangles[(ct + loading_object_current_faces_count) * ts + 2], &triangles[(ct + loading_object_current_faces_count) * ts + 8], &triangles[(ct + loading_object_current_faces_count) * ts + 5]);

            if (matches != 9){
                triangles[(ct + loading_object_current_faces_count) * ts + 0] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 1] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 2] = -100;
                
                triangles[(ct + loading_object_current_faces_count) * ts + 3] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 4] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 5] = -100;
                
                triangles[(ct + loading_object_current_faces_count) * ts + 6] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 7] = -100;
                triangles[(ct + loading_object_current_faces_count) * ts + 8] = -100;

                if (strchr(line, '/')){
                    obj_face_type = 1;

                    sscanf(line, "f %d//%d %d//%d %d//%d", 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 0], &triangles[(ct + loading_object_current_faces_count) * ts + 3], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 1], &triangles[(ct + loading_object_current_faces_count) * ts + 4], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 2], &triangles[(ct + loading_object_current_faces_count) * ts + 5]);
                }
                else {
                    obj_face_type = 2;

                    sscanf(line, "f %d %d %d", 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 0], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 1], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 2]);
                }
            }

            ct++;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            sscanf(line, "v %f %f %f", 
                &verticies[(cv + loading_object_current_verticies_count) * vs + 0], 
                &verticies[(cv + loading_object_current_verticies_count) * vs + 1], 
                &verticies[(cv + loading_object_current_verticies_count) * vs + 2]);

            cv++;
        } 
        else if (line[0] == 'v' && line[1] == 'n') {
            sscanf(line, "vn %f %f %f", 
                &normals[(cn + loading_object_current_normals_count) * vs + 0], 
                &normals[(cn + loading_object_current_normals_count) * vs + 1], 
                &normals[(cn + loading_object_current_normals_count) * vs + 2]);

            cn++;
        } 
        else if (line[0] == 'v' && line[1] == 't') {
            sscanf(line, "vt %f %f %f", 
                &uvs[(cu + loading_object_current_uvs_count) * vs + 0], 
                &uvs[(cu + loading_object_current_uvs_count) * vs + 1], 
                &uvs[(cu + loading_object_current_uvs_count) * vs + 2]);

            cu++;
        } 
    }

    switch(obj_face_type){
        case 0:
            debug(RI_DEBUG_MEDIUM, "OBJ file uses X/X/X X/X/X X/X/X");
            break;
            
        case 1:
            debug(RI_DEBUG_MEDIUM, "OBJ file uses X//X X//X X//X");
            break;

        case 2:
            debug(RI_DEBUG_MEDIUM, "OBJ file uses X X X");
            break;
        
        default:
            break;
    }

    debug(RI_DEBUG_MEDIUM, "%d Triangles", ct);
    debug(RI_DEBUG_MEDIUM, "%d Verticies", cv);
    debug(RI_DEBUG_MEDIUM, "%d Normals", cn);
    debug(RI_DEBUG_MEDIUM, "%d UVS", cu);

    objects[base].modelInfo.triangleCount = ct; // triangle count
    objects[base].modelInfo.vertexCount = cv; // vertex count
    objects[base].modelInfo.normalCount = cn; // normal count
    objects[base].modelInfo.uvCount = cu; // uv count

    fclose(file);

    load_object_return return_values = {
        0, 0, 0, 0, 0
    };

    debug(RI_DEBUG_HIGH, "Left load_object");

    return return_values;
}

RI_objects RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest){
    debug(RI_DEBUG_HIGH, "Called RI_RequestObjects");
    
    debug(RI_DEBUG_MEDIUM, "Requesting %d Objects...", object_count);

    int object_arary_size = sizeof(Object) * (object_count + RI_ObjectsToRequest);

    debug(RI_DEBUG_MEDIUM, "object_arary_size is %d bytes", object_arary_size);

    RI_objects temp = realloc(objects, object_arary_size);
    
    if (temp == NULL){
        debug(RI_DEBUG_LOW, "Realloc Error for Objects Array");
    }
    else{
        objects = temp;
    }

    char **file_names = malloc(RI_ObjectsToRequest * sizeof(char *));
    char **allocated_file_names = malloc(RI_ObjectsToRequest * sizeof(char *));
    char **texture_names = malloc(RI_ObjectsToRequest * sizeof(char *));
    int *object_file_offsets = malloc(RI_ObjectsToRequest * sizeof(int) * 5);

    for (int i_object = 0; i_object < RI_ObjectsToRequest; i_object++){
        file_names[i_object] = RI_ObjectBuffer[i_object].file_path;
        texture_names[i_object] = "blahblahblah placeholder (this is some salt)";
        allocated_file_names[i_object] = "blahblahblah placeholder (this is some salt)";
        debug(RI_DEBUG_HIGH, "file_names[object] is %s", file_names[i_object]);
        texture_names[i_object] = "blahblahblah placeholder (this is some salt)";
    }

    int loaded_object_files = malloc_objects(RI_ObjectsToRequest, file_names, allocated_file_names, object_file_offsets);
    
    free(file_names);
    free(allocated_file_names);

    texture_count = 0;
    int is_this_texture_name_already_in_the_texture_names_array = 0;

    textures_size = 0;

    int transform_vertex_offset_total = 0;
    int transform_normal_offset_total = 0;

    for (int i_object = 0; i_object < RI_ObjectsToRequest; i_object++){        
        RI_newObject *loading_object_current_object = &RI_ObjectBuffer[i_object];
        
        int base = object_count + i_object;

        objects[base].modelInfo.triangleOffset = (float)object_file_offsets[i_object * 5 + 0]; // triangle offset
        objects[base].modelInfo.vertexOffset = (float)object_file_offsets[i_object * 5 + 1]; // vertex offset
        objects[base].modelInfo.normalOffset = (float)object_file_offsets[i_object * 5 + 2]; // normal offset
        objects[base].modelInfo.uvOffset = (float)object_file_offsets[i_object * 5 + 3]; // uvs offset
        objects[base].material.properties = loading_object_current_object->material_flags;

        is_this_texture_name_already_in_the_texture_names_array = 0;

        for (int i_object_texture = 0; i_object_texture < RI_ObjectsToRequest; i_object_texture++){
            if (strcmp(texture_names[i_object_texture], loading_object_current_object->texture) == 0){
                debug(RI_DEBUG_HIGH, "Not Reloading Texture \"%s\" (texture #%d) (compared %s to %s)", loading_object_current_object->texture, i_object_texture, texture_names[i_object_texture], loading_object_current_object->texture);

                objects[base].material.textureOffset = i_object_texture; // texture offset

                is_this_texture_name_already_in_the_texture_names_array = 1;
                break;
            }
        }    

        if (!is_this_texture_name_already_in_the_texture_names_array){
            texture_names[texture_count] = loading_object_current_object->texture;
            
            objects[base].material.textureOffset = texture_count; // texture offset

            texture_count++;

            int texture_width, texture_height, channels;

            unsigned char *temp_image = stbi_load(loading_object_current_object->texture, &texture_width, &texture_height, &channels, 4);
           
            if(stbi_failure_reason()){
                texture_width = 1;
                texture_height = 1;

                debug(RI_DEBUG_LOW, "Error Loading Texture \"%s\"", loading_object_current_object->texture);
            }
            else{
                debug(RI_DEBUG_MEDIUM, "Texture (%s) Loaded With Size %dx%d (%d channels)", loading_object_current_object->texture, texture_width, texture_height, channels);
            }

            textures_size += texture_width * texture_height;

            stbi_image_free(temp_image);
        }

        loading_object_current_faces_count = object_file_offsets[i_object * 5 + 0];
        loading_object_current_verticies_count = object_file_offsets[i_object * 5 + 1];
        loading_object_current_normals_count = object_file_offsets[i_object * 5 + 2];
        loading_object_current_uvs_count = object_file_offsets[i_object * 5 + 3];

        if (object_file_offsets[i_object * 5 + 4] > 0){
            debug(RI_DEBUG_HIGH, "Loading Object at Triangle Index: %d", objects[base].modelInfo.triangleOffset);
            debug(RI_DEBUG_HIGH, "Loading Object at Vertex Index: %d", objects[base].modelInfo.vertexOffset);
            debug(RI_DEBUG_HIGH, "Loading Object at Normal Index: %d", objects[base].modelInfo.normalOffset);
            debug(RI_DEBUG_HIGH, "Loading Object at UV Index: %d", objects[base].modelInfo.uvOffset);

            load_object((char *)loading_object_current_object->file_path, i_object, base);
        }
        else{
            debug(RI_DEBUG_HIGH, "Object Already Loaded at Triangle Index: %d", objects[base].modelInfo.triangleOffset);
            debug(RI_DEBUG_HIGH, "Object Already Loaded at Vertex Index: %d", objects[base].modelInfo.vertexOffset);
            debug(RI_DEBUG_HIGH, "Object Already Loaded at Normal Index: %d", objects[base].modelInfo.normalOffset);
            debug(RI_DEBUG_HIGH, "Object Already Loaded at UV Index: %d", objects[base].modelInfo.uvOffset);
        
            objects[base].modelInfo.triangleCount = objects[-object_file_offsets[i_object * 5 + 4] - 1].modelInfo.triangleCount;
            objects[base].modelInfo.vertexCount = objects[-object_file_offsets[i_object * 5 + 4] - 1].modelInfo.vertexCount;
            objects[base].modelInfo.normalCount = objects[-object_file_offsets[i_object * 5 + 4] - 1].modelInfo.normalCount;

            debug(RI_DEBUG_HIGH, "Object Already Loaded Has %d Triangles", objects[base].modelInfo.triangleCount);
        }

        objects[base].transform.position.x = loading_object_current_object->x; // x
        objects[base].transform.position.y = loading_object_current_object->y; // y
        objects[base].transform.position.z = loading_object_current_object->z; // z
        objects[base].transform.rotation.w = loading_object_current_object->r_w; // rotation x
        objects[base].transform.rotation.x = loading_object_current_object->r_x; // rotation y
        objects[base].transform.rotation.y = loading_object_current_object->r_y; // rotation z
        objects[base].transform.rotation.z = loading_object_current_object->r_z; // rotation w
        objects[base].transform.scale.x = loading_object_current_object->s_x; // scale x
        objects[base].transform.scale.y = loading_object_current_object->s_y; // scale y
        objects[base].transform.scale.z = loading_object_current_object->s_z; // scale z

        objects[base].modelInfo.transformedVertexOffset = transform_vertex_offset_total;
        transform_vertex_offset_total += objects[base].modelInfo.vertexCount;
    
        objects[base].modelInfo.transformedNormalOffset = transform_normal_offset_total;
        transform_normal_offset_total += objects[base].modelInfo.normalCount;
    
        debug(0, "%d, %d", transform_vertex_offset_total, objects[base].modelInfo.vertexCount);
    }
    
    free(object_file_offsets);

    debug(RI_DEBUG_MEDIUM, "Allocating Texture Buffers");

    textures = malloc(sizeof(unsigned char) * textures_size * 4);
    texture_info = malloc(sizeof(int) * tis * texture_count);

    if (!textures){
        debug(RI_DEBUG_MEDIUM, "Failed to Allocate Texture Buffer");
    }

    if (!texture_info){
        debug(RI_DEBUG_MEDIUM, "Failed to Allocate Texture Info Buffer");
    }

    int value_offset = 0;
    for (int i_current_texture = 0; i_current_texture < texture_count; i_current_texture++){
        char *current_texture_name = texture_names[i_current_texture];

        int temp_width, temp_height;
        RI_textures temp_texture = stbi_load(current_texture_name, &temp_width, &temp_height, NULL, 4);

        if(stbi_failure_reason()){
            temp_width = 1;
            temp_height = 1;
            
            textures[0 + value_offset] = 255;
            textures[1 + value_offset] = 0;
            textures[2 + value_offset] = 255;
            textures[3 + value_offset] = 128;
        }
        else {        
            texture_info[i_current_texture * tis] = temp_width;
            texture_info[i_current_texture * tis + 1] = temp_height;
            texture_info[i_current_texture * tis + 2] = value_offset;

            debug(RI_DEBUG_HIGH, "Texture Info for Texture #%d: width: %d, height: %d, offset: %d", i_current_texture, texture_info[i_current_texture * tis], texture_info[i_current_texture * tis + 1], texture_info[i_current_texture * tis + 2]);

            for (int i_current_value = 0; i_current_value < temp_width * temp_height * 4; i_current_value++){
                textures[i_current_value + value_offset] = temp_texture[i_current_value];
            }
        }
        
        value_offset += temp_width * temp_height * 4;
        
        stbi_image_free(temp_texture);
    }

    if (!use_cpu && texture_count > 0){
        textures_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned char) * textures_size * 4, textures, &error);
        erchk(error);
    
        if (textures_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Textures cl_mem Buffer");
        }

        texture_info_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * tis * texture_count, texture_info, &error);
        erchk(error);
    
        if (texture_info_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Texture Info cl_mem Buffer");
        }

        erchk(clEnqueueWriteBuffer(queue, textures_memory_buffer, CL_TRUE, 0, sizeof(unsigned char) * 4 * textures_size, textures, 0, NULL, NULL));
        erchk(clFinish(queue));

        erchk(clEnqueueWriteBuffer(queue, texture_info_memory_buffer, CL_TRUE, 0, sizeof(int) * tis * texture_count, texture_info, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote Textures Buffer and Texture Info Buffer");
    }

    for (int i = 0; i < face_count * 9; i++){
        triangles[i]--;
    }
    
    size_t texture_bytes   = sizeof(unsigned char) * textures_size * 4;
    size_t triangle_bytes  = sizeof(RI_triangles) * face_count * ts;
    size_t split_triangle_bytes  = sizeof(int) * face_count;
    size_t vertex_bytes    = sizeof(RI_verticies) * vertex_count * vs;
    size_t transformed_vertex_bytes    = sizeof(RI_verticies) * transform_vertex_offset_total * transformed_verticies_size * use_cpu;
    size_t split_vertex_bytes    = sizeof(RI_verticies) * face_count * transformed_verticies_size * use_cpu;
    size_t normal_bytes    = sizeof(RI_verticies) * normal_count * vs;
    size_t transformed_normal_bytes    = sizeof(RI_verticies) * transform_normal_offset_total * vs * use_cpu;
    size_t uv_bytes        = sizeof(RI_verticies) * uv_count * vs;
    size_t total_bytes     = texture_bytes + triangle_bytes + vertex_bytes + transformed_vertex_bytes + split_vertex_bytes + normal_bytes + transformed_normal_bytes + uv_bytes + object_arary_size;

    debug(RI_DEBUG_MEDIUM,
        "Allocated %zu Bytes for Objects (%d Textures (%zu Bytes), "
        "%d Triangles (%zu Original & %zu Split Bytes), %d Vertices (%zu Original, %zu Transformed, & %zu Split Bytes), "
        "%d Normals (%zu Original & %zu Transformed Bytes), %d UVs (%zu Bytes), "
        "%d Objects (%zu Bytes))",
        total_bytes,
        texture_count, texture_bytes,
        face_count, triangle_bytes, split_triangle_bytes,
        vertex_count, vertex_bytes, transformed_vertex_bytes, split_vertex_bytes,
        normal_count, normal_bytes, transformed_normal_bytes,
        uv_count, uv_bytes,
        RI_ObjectsToRequest, object_arary_size);

    debug(RI_DEBUG_HIGH, "clCreateBuffer object_arary_size: %d", object_arary_size);

    if (!use_cpu){
        object_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, object_arary_size, objects, &error);
        erchk(error);
    }

    if (!use_cpu && object_memory_buffer == NULL){
        debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Objects cl_mem Buffer");
    }

    if (!use_cpu && face_count > 0){
        triangles_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_triangles) * face_count * ts, triangles, &error);
        erchk(error);
    
        if (triangles_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Triangles cl_mem Buffer");
        }

        erchk(clEnqueueWriteBuffer(queue, triangles_memory_buffer, CL_TRUE, 0, sizeof(int) * ts * face_count, triangles, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote Triangles Buffer");
    }

    float zero = 0.0f;
    
    if (!use_cpu && vertex_count > 0){
        debug(1, "transform_vertex_offset_total %d", transform_vertex_offset_total);

        verticies_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * vertex_count * vs, verticies, &error);
        erchk(error);
        transformed_verticies_memory_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(RI_verticies) * transform_vertex_offset_total * vs, NULL, &error);
        erchk(error);
    
        if (verticies_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Verticies cl_mem Buffer");
        }

        if (transformed_verticies_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Verticies Transform cl_mem Buffer");
        }   

        erchk(clEnqueueWriteBuffer(queue, verticies_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * vertex_count, verticies, 0, NULL, NULL));
        erchk(clFinish(queue));


        erchk(clEnqueueFillBuffer(queue, transformed_verticies_memory_buffer, &zero, sizeof(float), 0, sizeof(RI_verticies) * transform_vertex_offset_total * vs * 2, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote Verticies Buffer");
    }

    if (!use_cpu && normal_count > 0){
        debug(1, "transform_normal_offset_total %d", transform_normal_offset_total);

        normals_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * normal_count * vs, normals, &error);
        erchk(error);
        transformed_normals_memory_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(RI_verticies) * transform_normal_offset_total * vs, NULL, &error);
        erchk(error);
    
        if (normals_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Normals cl_mem Buffer");
        }
        if (transformed_normals_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Normals Transform cl_mem Buffer");
        }

        erchk(clEnqueueWriteBuffer(queue, normals_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * normal_count, normals, 0, NULL, NULL));
        erchk(clFinish(queue));

        erchk(clEnqueueFillBuffer(queue, transformed_normals_memory_buffer, &zero, sizeof(float), 0, sizeof(RI_verticies) * transform_normal_offset_total * vs, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote Normals Buffer");
    }

    if (!use_cpu && uv_count > 0){
        uvs_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * uv_count * vs, uvs, &error);
        erchk(error);

        if (uvs_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for UVS cl_mem Buffer");
        }

        erchk(clEnqueueWriteBuffer(queue, uvs_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * uv_count, uvs, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote UVS Buffer");
    }

    if (use_cpu){
        transformed_verticies = malloc(transformed_vertex_bytes);
        transformed_normals = malloc(transformed_normal_bytes);
        split_verticies = malloc(split_vertex_bytes);
        split_triangles = malloc(split_vertex_bytes);
    }

    debug(RI_DEBUG_MEDIUM, "Request for %d Objects Granted", RI_ObjectsToRequest);
    
    debug(RI_DEBUG_HIGH, "Left RI_RequestObjects");

    free(texture_names);

    object_count += RI_ObjectsToRequest;    

    return objects;
}

RI_result RI_SetFpsCap(int RI_FpsCap){
    debug(RI_DEBUG_HIGH, "Called RI_SetFpsCap");

    fps_cap = RI_FpsCap;

    return RI_SUCCESS;
}

RI_result RI_SetFontColor(RI_uint RI_FontColorRGBA){
    debug(RI_DEBUG_HIGH, "Called RI_SetFontColor");

    font_color.r = (RI_FontColorRGBA >> 24) & 0xFF;
    font_color.g = (RI_FontColorRGBA >> 16) & 0xFF;
    font_color.b = (RI_FontColorRGBA >> 8) & 0xFF;
    font_color.a = RI_FontColorRGBA & 0xFF;

    return RI_SUCCESS;
}

RI_result RI_SetFontSize(int RI_FontSize){
    debug(RI_DEBUG_HIGH, "Called RI_SetFontSize");

    font_size = RI_FontSize;
    
    return RI_SUCCESS;    
}

RI_result RI_SetFontFile(char *RI_PathToFontFile){
    debug(RI_DEBUG_HIGH, "Called RI_SetFontFile");

    TTF_Font* font_check = TTF_OpenFont(RI_PathToFontFile, 24);

    if (font_check == NULL){
        return RI_ERROR;
    }

    font_file = RI_PathToFontFile;

    font = TTF_OpenFont(font_file, font_size);

    return RI_SUCCESS;
}

RI_result RI_SetDebugPrefix(char RI_Prefix[50]){
    strcpy(prefix, RI_Prefix);

    return RI_SUCCESS;
}
// ----- Set Value Functions


int is_intersecting(float a, float b, float c, float d, float p, float q, float r, float s) { 
    float det, gamma, lambda; 
    
    det = (c - a) * (s - q) - (r - p) * (d - b); 
    
    if (det == 0) { 
        return 1; 
    }  
    else { 
        lambda = ((s - q) * (r - a) + (p - r) * (s - b)) / det; 
        gamma = ((b - d) * (r - a) + (c - a) * (s - b)) / det; 
        return (0 < lambda && lambda < 1) && (0 < gamma && gamma < 1); 
    } 
} 

void norm(float dest[2], float a[2]){ 
    float magnitude = sqrt(a[0] * a[0] + a[1] * a[1]); 
    
    dest[0] = a[0] / magnitude; 
    dest[1] = a[1] / magnitude; 
    } 
    
    void sub(float dest[2], float a[2], float b[2]){ 
    dest[0] = a[0] - b[0]; 
    dest[1] = a[1] - b[1]; 
    } 
    
    void add(float dest[2], float a[2], float b[2]){ 
    dest[0] = a[0] + b[0]; 
    dest[1] = a[1] + b[1]; 
} 

int clamppp(int x, int lower, int upper) {
    return x < lower ? lower : (x > upper ? upper : x);
}

float clamppp_float(float x, float lower, float upper) {
    return x < lower ? lower : (x > upper ? upper : x);
}

void rotate_quaternion(float *x, float *y, float *z, float r_x, float r_y, float r_z){

};

void rotate_euler(float *x, float *y, float *z, float r_x, float r_y, float r_z){
    float cx = cos(r_x), sx = sin(r_x);
    float cy = cos(r_y), sy = sin(r_y);
    float cz = cos(r_z), sz = sin(r_z);

    float matrix[3][3] = {
        {
            cy * cz,
            -cy * sz,
            sy
        },
        {
            sx * sy * cz + cx * sz,
            -sx * sy * sz + cx * cz,
            -sx * cy
        },
        {
            -cx * sy * cz + sx * sz,
            cx * sy * sz + sx * cz,
            cx * cy
        }
    };
    
    float temp_x = matrix[0][0] * *x + matrix[0][1] * *y + matrix[0][2] * *z;
    float temp_y = matrix[1][0] * *x + matrix[1][1] * *y + matrix[1][2] * *z;
    float temp_z = matrix[2][0] * *x + matrix[2][1] * *y + matrix[2][2] * *z;

    *x = temp_x;
    *y = temp_y;
    *z = temp_z;
};

// P0: correct point, point we're lerping to (first 3)
// P1 & P2: points that lerp (last 12)
void clip_tri_shrink(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float *nx1, float *ny1, float *nz1, float *nx2, float *ny2, float *nz2, float *frac_0, float *frac_2){
    float percent_clipped_0 = (near_clip - z0) / (z1 - z0);
    float percent_clipped_1 = (near_clip - z0) / (z2 - z0);
    
    *frac_0 = percent_clipped_0;
    *frac_2 = percent_clipped_1;

    Vec2 pos1 = {x0, y0};
    Vec2 pos2 = {x1, y1};

    Vec2 p1 = lerp(pos1, pos2, percent_clipped_0);

    pos2.x = x2;
    pos2.y = y2;

    Vec2 p2 = lerp(pos1, pos2, percent_clipped_1);

    *nx1 = p1.x;
    *ny1 = p1.y;

    *nx2 = p2.x;
    *ny2 = p2.y;

    *nz1 = near_clip;
    *nz2 = near_clip;
}

// P0 & P1 are okay
// P2 is clipped
void clip_tri_split(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float *nx, float *ny, float *nz, float *tx0, float *ty0, float *tz0, float *tx1, float *ty1, float *tz1, float *tx2, float *ty2, float *tz2, float *frac_1, float *frac_2, float *tfrac_1){
    float percent_clipped_0 = (near_clip - z2) / (z0 - z2);
    float percent_clipped_1 = (near_clip - z2) / (z1 - z2);
    
    *frac_1 = percent_clipped_1;
    *frac_2 = percent_clipped_0;
    *tfrac_1 = percent_clipped_1;

    Vec2 pos1 = {x2, y2};
    Vec2 pos2 = {x0, y0};

    Vec2 p2_tp0 = lerp(pos1, pos2, percent_clipped_0);

    pos2.x = x1;
    pos2.y = y1;

    Vec2 tp2 = lerp(pos1, pos2, percent_clipped_1);

    *nx = p2_tp0.x;
    *ny = p2_tp0.y;

    *tx0 = *nx;
    *ty0 = *ny;

    *tx2 = tp2.x;
    *ty2 = tp2.y;

    *nz = near_clip;
    *tz0 = near_clip;
    *tz2 = near_clip;

    *tx1 = x1;
    *ty1 = y1;
    *tz1 = z1;
}

// ----- Renderer Action Functions
RI_result RI_Tick(){
    if (show_fps || debug_fps){
        start_time = SDL_GetPerformanceCounter();
    }
    
    debug_tick_func(1, "Called RI_Tick");
    
    if (running)
    {
        if (frame_buffer == NULL)
        {
            debug_tick_func(0, "Frame Buffer is not Allocated");
            return RI_ERROR;
        } 

        if (use_cpu){
            for (int base = 0; base < object_count; base++){
            
                float vertical_fov_factor = ri_height / tanf(0.5 * fov);
            float horizontal_fov_factor = ri_width / tanf(0.5 * fov);

    
    float object_x =   objects[base].transform.position.x;
    float object_y =   objects[base].transform.position.y;
    float object_z =   objects[base].transform.position.z;
    float object_r_x = objects[base].transform.rotation.w;
    float object_r_y = objects[base].transform.rotation.x;
    float object_r_z = objects[base].transform.rotation.y;
    float object_r_w = objects[base].transform.rotation.z;
    float object_s_x = objects[base].transform.scale.x; 
    float object_s_y = objects[base].transform.scale.y; 
    float object_s_z = objects[base].transform.scale.z; 
    
    int triangle_count = objects[base].modelInfo.triangleCount;
    int triangle_index = objects[base].modelInfo.triangleOffset;
    int vertex_index =   objects[base].modelInfo.vertexOffset;
    int transformed_vertex_index = objects[base].modelInfo.transformedVertexOffset;
    int normal_index =   objects[base].modelInfo.normalOffset;
    int transformed_normal_index =   objects[base].modelInfo.transformedNormalOffset;
    
    for (int i_triangle = 0; i_triangle < triangle_count; i_triangle++){
        int triangle_base = (i_triangle + triangle_index) * 9; 
        
        int i0 = (vertex_index + triangles[triangle_base + 0]) * 3;
        int i1 = (vertex_index + triangles[triangle_base + 1]) * 3;
        int i2 = (vertex_index + triangles[triangle_base + 2]) * 3;
        
        int i3 = (normal_index + triangles[triangle_base + 3]) * 3;
        int i4 = (normal_index + triangles[triangle_base + 4]) * 3;
        int i5 = (normal_index + triangles[triangle_base + 5]) * 3;
        
        float x0 = verticies[i0 + 0] * object_s_x;
        float y0 = verticies[i0 + 1] * object_s_y;
        float z0 = verticies[i0 + 2] * object_s_z;
        float x1 = verticies[i1 + 0] * object_s_x;
        float y1 = verticies[i1 + 1] * object_s_y;
        float z1 = verticies[i1 + 2] * object_s_z;
        float x2 = verticies[i2 + 0] * object_s_x;
        float y2 = verticies[i2 + 1] * object_s_y;
        float z2 = verticies[i2 + 2] * object_s_z;
        float n_x0 = normals[i3 + 0];
        float n_y0 = normals[i3 + 1];
        float n_z0 = normals[i3 + 2];
        float n_x1 = normals[i4 + 0];
        float n_y1 = normals[i4 + 1];
        float n_z1 = normals[i4 + 2];
        float n_x2 = normals[i5 + 0];
        float n_y2 = normals[i5 + 1];
        float n_z2 = normals[i5 + 2];
        
        if (i3 < 0 || i4 < 0 || i5 < 0){
            //   has_normals = 0;
        }
        
        if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){
            continue;
        }
        
        rotate_euler(&x0, &y0, &z0, object_r_x, object_r_y, object_r_z);
        rotate_euler(&x1, &y1, &z1, object_r_x, object_r_y, object_r_z);
        rotate_euler(&x2, &y2, &z2, object_r_x, object_r_y, object_r_z);
        
        rotate_euler(&n_x0, &n_y0, &n_z0, object_r_x, object_r_y, object_r_z);
        rotate_euler(&n_x1, &n_y1, &n_z1, object_r_x, object_r_y, object_r_z);
        rotate_euler(&n_x2, &n_y2, &n_z2, object_r_x, object_r_y, object_r_z);
        
        z0 = (z0 + object_z);
        x0 = (x0 + object_x);
        y0 = (y0 + object_y);
        z1 = (z1 + object_z);
        x1 = (x1 + object_x);
        y1 = (y1 + object_y);
        z2 = (z2 + object_z);
        x2 = (x2 + object_x);
        y2 = (y2 + object_y);
        
        
        float clipped_z0 = z0;
        float clipped_x0 = x0;
        float clipped_y0 = y0;
        float clipped_z1 = z1;
        float clipped_x1 = x1;
        float clipped_y1 = y1;
        float clipped_z2 = z2;
        float clipped_x2 = x2;
        float clipped_y2 = y2;

        int clip_z0 = z0 <= near_clip;
                            int clip_z1 = z1 <= near_clip;
                            int clip_z2 = z2 <= near_clip;
                            
                            int clip_count = clip_z0 + clip_z1 + clip_z2;

                            split_triangles[i_triangle] = -1;


                            float *frac_0 = &transformed_verticies[i_triangle * transformed_verticies_size + 9]; // p0 to p1
                            float *frac_1 = &transformed_verticies[i_triangle * transformed_verticies_size + 10]; // p1 to p2
                            float *frac_2 = &transformed_verticies[i_triangle * transformed_verticies_size + 11]; // p2 to p0

if (selected_triangle >= 0 && selected_triangle != i_triangle)continue;
                            switch (clip_count){
                                case 0:{ // do nothing, they are all okay!! >w<


                                    break;}
                                
                                case 1:{ // split triangle into 2
                                    float *tx0 = &split_verticies[i_triangle * transformed_verticies_size + 0];
                                    float *ty0 = &split_verticies[i_triangle * transformed_verticies_size + 1];
                                    float *tz0 = &split_verticies[i_triangle * transformed_verticies_size + 2];
                                    float *tx1 = &split_verticies[i_triangle * transformed_verticies_size + 3];
                                    float *ty1 = &split_verticies[i_triangle * transformed_verticies_size + 4];
                                    float *tz1 = &split_verticies[i_triangle * transformed_verticies_size + 5];
                                    float *tx2 = &split_verticies[i_triangle * transformed_verticies_size + 6];
                                    float *ty2 = &split_verticies[i_triangle * transformed_verticies_size + 7];
                                    float *tz2 = &split_verticies[i_triangle * transformed_verticies_size + 8];
                                    float *tfrac_0 = &split_verticies[i_triangle * transformed_verticies_size + 9];
                                    float *tfrac_1 = &split_verticies[i_triangle * transformed_verticies_size + 10];
                                    float *tfrac_2 = &split_verticies[i_triangle * transformed_verticies_size + 11];

                                    *frac_0 = 0;
                                    *frac_1 = 0;
                                    *frac_2 = 0;
                                    *tfrac_0 = 0;
                                    *tfrac_1 = 0;
                                    *tfrac_2 = 0;
                                    
                                    if (clip_z0){
                                        clip_tri_split(x2, y2, z2,x1, y1, z1,  x0, y0, z0, &clipped_x0, &clipped_y0, &clipped_z0,     tx1, ty1, tz1,     tx2, ty2, tz2,     tx0, ty0, tz0,   frac_2, frac_0,  tfrac_2);
                                    }
                                    
                                    if (clip_z1){
                                        clip_tri_split(x2, y2, z2, x0, y0, z0, x1, y1, z1, &clipped_x1, &clipped_y1, &clipped_z1,     tx2, ty2, tz2,     tx0, ty0, tz0,     tx1, ty1, tz1,   frac_0, frac_1,   tfrac_0);
                                    }

                                    if (clip_z2){
                                        clip_tri_split(x0, y0, z0, x1, y1, z1, x2, y2, z2, &clipped_x2, &clipped_y2, &clipped_z2,     tx0, ty0, tz0,     tx1, ty1, tz1,     tx2, ty2, tz2,   frac_1, frac_2,  tfrac_1);
                                    }

                                    *tx0 = *tx0 / *tz0 * horizontal_fov_factor;
                                    *ty0 = *ty0 / *tz0 * vertical_fov_factor;
                                    *tx1 = *tx1 / *tz1 * horizontal_fov_factor;
                                    *ty1 = *ty1 / *tz1 * vertical_fov_factor;
                                    *tx2 = *tx2 / *tz2 * horizontal_fov_factor;
                                    *ty2 = *ty2 / *tz2 * vertical_fov_factor;

                                    split_triangles[i_triangle] = 0;

                                    objects[base].split_triangles++;

                                    break;
                                }

                                case 2:{ // shrink triangle into 1
                                    
                                    if (!clip_z0){ // z0 is fine
                                        clip_tri_shrink(x0, y0, z0, x1, y1, z1, x2, y2, z2, &clipped_x1, &clipped_y1, &clipped_z1, &clipped_x2, &clipped_y2, &clipped_z2,   frac_0, frac_2);
                                        *frac_1 = 0;

                                        break;
                                    }

                                    if (!clip_z1){
 //                                                                                                                                                                         0 >1    2 > 1
                                        clip_tri_shrink(x1, y1, z1, x0, y0, z0, x2, y2, z2, &clipped_x0, &clipped_y0, &clipped_z0, &clipped_x2, &clipped_y2, &clipped_z2,   frac_0, frac_1);
                                        *frac_2 = 0;
                                    
                                        break;
                                    }

                                    if (!clip_z2){
//                                                                                                                                                                          0 > 2   1 > 2
                                        clip_tri_shrink(x2, y2, z2, x0, y0, z0, x1, y1, z1, &clipped_x0, &clipped_y0, &clipped_z0, &clipped_x1, &clipped_y1, &clipped_z1,   frac_2, frac_1);
                                        *frac_0 = 0;
                                    
                                        break;
                                    }


                                    break;
                                }

                                case 3:{ // lost cause, exit
            // transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 0] = 10561923;
                                 
                            split_triangles[i_triangle] = -2;
                                 
                                    continue;
                                }

                                default: { // shouldn't happen
                                    continue;
                                }
                            }
                            

        
                            clipped_x0 = clipped_x0 / clipped_z0 * horizontal_fov_factor;
                            clipped_y0 = clipped_y0 / clipped_z0 * vertical_fov_factor;
                            clipped_x1 = clipped_x1 / clipped_z1 * horizontal_fov_factor;
                            clipped_y1 = clipped_y1 / clipped_z1 * vertical_fov_factor;
                            clipped_x2 = clipped_x2 / clipped_z2 * horizontal_fov_factor;
                            clipped_y2 = clipped_y2 / clipped_z2 * vertical_fov_factor;

        // if ((x0 < -ri_h_width && x1 < -ri_h_width && x2 < -ri_h_width) || (y0 < -ri_h_height && y1 < -ri_h_height && y2 < -ri_h_height) || (x0 >= ri_h_width && x1 >= ri_h_width && x2 >= ri_h_width) || (y0 >= ri_h_height && y1 >= ri_h_height && y2 >= ri_h_height)){
            // transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 0] = 999999;
        // }
        // else{
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 0] = clipped_x0;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 1] = clipped_y0;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 2] = clipped_z0;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 3] = clipped_x1;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 4] = clipped_y1;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 5] = clipped_z1;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 6] = clipped_x2;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 7] = clipped_y2;
            transformed_verticies[(i_triangle + transformed_vertex_index) * transformed_verticies_size + 8] = clipped_z2;
            
            transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 0] = n_x0;
            transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 1] = n_y0;
            transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 2] = n_z0;
            transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 0] = n_x1;
            transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 1] = n_y1;
            transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 2] = n_z1;
            transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 0] = n_x2;
            transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 1] = n_y2;
            transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 2] = n_z2;
        // }
    }}
            for (int id_y = -ri_h_height; id_y < ri_h_height; id_y++){
                for (int id_x = -ri_h_width; id_x < ri_h_width; id_x++){
                    float z_pixel = INFINITY; 
                    unsigned int frame_pixel = 0x22222222; 
                    
                    float highest_z = 800;
                    float lowest_z = 0;
                    
                    int has_normals = 1;
                    int has_uvs = 1;
                    
                    float w0;
                    float w1;
                    float w2;
                    
                    for (int i_object = 0; i_object < object_count; i_object++){ 
                        int base = i_object;

                        uint64_t material_flags = objects[base].material.properties;
                        ColorARGB albedo = objects[base].material.albedo;

                        int triangle_count = objects[base].modelInfo.triangleCount;
                        int triangle_index = objects[base].modelInfo.triangleOffset;
                        int vertex_index =   objects[base].modelInfo.vertexOffset;
                        int normal_index =   objects[base].modelInfo.normalOffset;
                        int uv_index =       objects[base].modelInfo.uvOffset;
                        int texture_index =  objects[base].material.textureOffset;
                        int transformed_vertex_index = objects[base].modelInfo.transformedVertexOffset;
                        int transformed_normal_index =   objects[base].modelInfo.transformedNormalOffset;
                        int split_triangles_count = objects[base].split_triangles;
                        
                        for (int i_triangle = 0; i_triangle < triangle_count; i_triangle++){    
                            albedo = objects[base].material.albedo;
                            if (selected_triangle >= 0 && i_triangle != selected_triangle){
                                continue;
                            }

                            if (split_triangles[i_triangle] == -2){
                                continue;
                            }

                            int triangle_base = (i_triangle + triangle_index) * 9; 

                            int i0 = i_triangle * transformed_verticies_size;

                            int i3 = (transformed_normal_index + triangles[triangle_base + 3]) * 3;
                            int i4 = (transformed_normal_index + triangles[triangle_base + 4]) * 3;
                            int i5 = (transformed_normal_index + triangles[triangle_base + 5]) * 3;

                            int i6 = (uv_index + triangles[triangle_base + 6]) * 3;
                            int i7 = (uv_index + triangles[triangle_base + 7]) * 3;
                            int i8 = (uv_index + triangles[triangle_base + 8]) * 3;
                            

                        
                        float x0 = transformed_verticies[i0 + 0]; 
                        float y0 = transformed_verticies[i0 + 1];
                        float z0 = transformed_verticies[i0 + 2];   
                        float x1 = transformed_verticies[i0 + 3];
                        float y1 = transformed_verticies[i0 + 4];
                        float z1 = transformed_verticies[i0 + 5];
                        float x2 = transformed_verticies[i0 + 6];
                        float y2 = transformed_verticies[i0 + 7];
                        float z2 = transformed_verticies[i0 + 8];
                        float frac_0 = transformed_verticies[i0 + 9];
                        float frac_1 = transformed_verticies[i0 + 10];
                        float frac_2 = transformed_verticies[i0 + 11];

                        if (i3 < 0 || i4 < 0 || i5 < 0){
                                has_normals = 0;
                            }
                            if (i6 < 0 || i7 < 0 || i8 < 0){
                                has_uvs = 0;
                            }
                            
                            if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){
                                continue;
                            }
                            
                            float smallest_x = x0; 
                            float largest_x = x0; 
                            float smallest_y = y0; 
                            float largest_y = y0; 
                            
                            if (x0 > largest_x) largest_x = x0;
                            if (x1 > largest_x) largest_x = x1;
                            if (x2 > largest_x) largest_x = x2;
                            
                            if (x0 < smallest_x) smallest_x = x0;
                            if (x1 < smallest_x) smallest_x = x1;
                            if (x2 < smallest_x) smallest_x = x2;
                            
                            if (y0 > largest_y) largest_y = y0;
                            if (y1 > largest_y) largest_y = y1;
                            if (y2 > largest_y) largest_y = y2;
                            
                            if (y0 < smallest_y) smallest_y = y0;
                            if (y1 < smallest_y) smallest_y = y1;
                            if (y2 < smallest_y) smallest_y = y2;
                            
                            smallest_x = fmin(smallest_x, 0); 
                            largest_x = fmax(largest_x, ri_width);  
                            smallest_y = fmin(smallest_y, 0); 
                            largest_y = fmax(largest_y, ri_height); 
                            
                            if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ 
                                float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); 
                                
                                if (!(material_flags & RI_MATERIAL_DOUBLE_SIDED) && denominator >= 0) { 
                                    continue; 
                                } 
                                
                                w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; 
                                w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y0)) / denominator; 
                                w2 = 1.0 - w0 - w1; 
                                
                                if (!(w0 > 0 && w1 > 0 && w2 > 0)){
                                    continue;
                                }

                                if (material_flags & RI_MATERIAL_WIREFRAME && (w0 >= wireframe_width && w1 >= wireframe_width && w2 >= wireframe_width)){
                                    continue;
                                }

                                float w_over_z = (w0 / z0 + w1 / z1 + w2 / z2); 
                                float z = 1.0 / w_over_z;

                                if (z < z_pixel){ 
                                    z_pixel = z; 
                                    
                                    float n_x0 = transformed_normals[i3 + 0];
                                    float n_y0 = transformed_normals[i3 + 1];
                                    float n_z0 = transformed_normals[i3 + 2];
                                    
                                    float n_x1 = transformed_normals[i4 + 0];
                                    float n_y1 = transformed_normals[i4 + 1];
                                    float n_z1 = transformed_normals[i4 + 2];
                                    
                                    float n_x2 = transformed_normals[i5 + 0];
                                    float n_y2 = transformed_normals[i5 + 1];
                                    float n_z2 = transformed_normals[i5 + 2];

                                    float u_x0 = frac_2 * uvs[i6 + 0] + (1.0 - frac_2) * uvs[i7 + 0];
                                    float u_y0 = frac_2 * uvs[i6 + 1] + (1.0 - frac_2) * uvs[i7 + 1];

                                    float u_x1 = frac_2 * uvs[i7 + 0] + (1.0 - frac_2) * uvs[i8 + 0];
                                    float u_y1 = frac_2 * uvs[i7 + 1] + (1.0 - frac_2) * uvs[i8 + 1];
                                                                
                                    float u_x2 = frac_2 * uvs[i8 + 0] + (1.0 - frac_2) * uvs[i6 + 0];
                                    float u_y2 = frac_2 * uvs[i8 + 1] + (1.0 - frac_2) * uvs[i6 + 1];
                                    
                                    switch (show_buffer){
                                        case 0:{
                                            if (!(material_flags & RI_MATERIAL_HAS_TEXTURE)){
                                                frame_pixel = (albedo.a << 24) | (albedo.r << 16) | (albedo.g << 8) | albedo.b;
                                            
                                                break;
                                            }

                                            double ux = (w0 * (u_x0 / z0) + w1 * (u_x1 / z1) + w2 * (u_x2 / z2)) / w_over_z;
                                            double uy = (w0 * (u_y0 / z0) + w1 * (u_y1 / z1) + w2 * (u_y2 / z2)) / w_over_z;
                                            
                                            int texture_width = texture_info[texture_index * 3];
                                            int texture_height = texture_info[texture_index * 3 + 1];
                                            int texture_value_offset = texture_info[texture_index * 3 + 2];
                                            
                                            int ix = fmax((int)(ux * texture_width), 0);
                                            int iy = fmax((int)(uy * texture_height), 0);
                                            
                                            int uv_pixel = (iy * texture_width + ix) * 4 + texture_value_offset;

                                            if (uv_pixel >= texture_width * texture_height * 4 + texture_value_offset)break;
                                            
                                            unsigned char r = textures[uv_pixel + 0];
                                            unsigned char g = textures[uv_pixel + 1];
                                            unsigned char b = textures[uv_pixel + 2];
                                            unsigned char a = textures[uv_pixel + 3];
                                            
                                            frame_pixel = (a << 24) | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 1:{
                                            float z = clamppp_float(z_pixel, 0.0f, highest_z);
                                            
                                            float norm_z = z / highest_z;
                                            
                                            unsigned char intensity = (unsigned char)(norm_z * 255.0f);
                                            
                                            frame_pixel = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;
                                            
                                            break;}
                                        case 2:{
                                            float nx = (w0 * (n_x0 / z0) + w1 * (n_x1 / z1) + w2 * (n_x2 / z2)) / w_over_z;
                                            float ny = (w0 * (n_y0 / z0) + w1 * (n_y1 / z1) + w2 * (n_y2 / z2)) / w_over_z;
                                            float nz = (w0 * (n_z0 / z0) + w1 * (n_z1 / z1) + w2 * (n_z2 / z2)) / w_over_z;
                                            
                                            nx = clamppp_float((nx * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            ny = clamppp_float((ny * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            nz = clamppp_float((nz * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            
                                            unsigned char r = (unsigned char)nx;
                                            unsigned char g = (unsigned char)ny;
                                            unsigned char b = (unsigned char)nz;
                                            
                                            if (!has_normals){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 3:{
                                            float ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;
                                            float uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;
                                            
                                            unsigned char r = (unsigned char)clamppp_float(ux * 255.0f, 0.0f, 255.0f);
                                            unsigned char g = (unsigned char)clamppp_float(uy * 255.0f, 0.0f, 255.0f);
                                            unsigned char b = 0;
                                            
                                            if (!has_uvs){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        default:{
                                            frame_pixel = 0xFF00FFFF;
                                            
                                            break;}
                                    }
                                } 
                            }
                        }

                        for (int i_triangle = 0; i_triangle < triangle_count; i_triangle++){      
                            albedo = objects[base].material.albedo;
                            if (selected_triangle >= 0 && i_triangle != selected_triangle){
                                continue;
                            }        

                            if (split_triangles[i_triangle] < 0){
                                continue;
                            }

                            int triangle_base = (i_triangle + triangle_index) * 9; 
                            
                            int i0 = i_triangle * transformed_verticies_size;

                            int i3 = (transformed_normal_index + triangles[triangle_base + 3]) * 3;
                            int i4 = (transformed_normal_index + triangles[triangle_base + 4]) * 3;
                            int i5 = (transformed_normal_index + triangles[triangle_base + 5]) * 3;

                            int i6 = (uv_index + triangles[triangle_base + 6]) * 3;
                            int i7 = (uv_index + triangles[triangle_base + 7]) * 3;
                            int i8 = (uv_index + triangles[triangle_base + 8]) * 3;
                            
                            float x0 = split_verticies[i0 + 0];
                            float y0 = split_verticies[i0 + 1];
                            float z0 = split_verticies[i0 + 2];   
                            float x1 = split_verticies[i0 + 3];
                            float y1 = split_verticies[i0 + 4];
                            float z1 = split_verticies[i0 + 5];
                            float x2 = split_verticies[i0 + 6]; 
                            float y2 = split_verticies[i0 + 7];
                            float z2 = split_verticies[i0 + 8];
                            
                        float frac_0 = split_verticies[i0 + 9];
                        float frac_1 = split_verticies[i0 + 10];
                        float frac_2 = split_verticies[i0 + 11];
                            if (i3 < 0 || i4 < 0 || i5 < 0){
                                has_normals = 0;
                            }
                            if (i6 < 0 || i7 < 0 || i8 < 0){
                                has_uvs = 0;
                            }
                            
                            if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){
                                continue;
                            }
                            
                            float smallest_x = x0; 
                            float largest_x = x0; 
                            float smallest_y = y0; 
                            float largest_y = y0; 
                            
                            if (x0 > largest_x) largest_x = x0;
                            if (x1 > largest_x) largest_x = x1;
                            if (x2 > largest_x) largest_x = x2;
                            
                            if (x0 < smallest_x) smallest_x = x0;
                            if (x1 < smallest_x) smallest_x = x1;
                            if (x2 < smallest_x) smallest_x = x2;
                            
                            if (y0 > largest_y) largest_y = y0;
                            if (y1 > largest_y) largest_y = y1;
                            if (y2 > largest_y) largest_y = y2;
                            
                            if (y0 < smallest_y) smallest_y = y0;
                            if (y1 < smallest_y) smallest_y = y1;
                            if (y2 < smallest_y) smallest_y = y2;
                            
                            smallest_x = fmin(smallest_x, 0); 
                            largest_x = fmax(largest_x, ri_width);  
                            smallest_y = fmin(smallest_y, 0); 
                            largest_y = fmax(largest_y, ri_height); 
                            
                            if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ 
                                float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); 
                                
                                w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; 
                                w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y0)) / denominator; 
                                w2 = 1.0 - w0 - w1; 
                                
                                if (!(w0 > 0 && w1 > 0 && w2 > 0)){
                                    continue;
                                }

                                if (material_flags & RI_MATERIAL_WIREFRAME && (w0 >= wireframe_width && w1 >= wireframe_width && w2 >= wireframe_width)){
                                    continue;
                                }

                                float w_over_z = (w0 / z0 + w1 / z1 + w2 / z2); 
                                float z = 1.0 / w_over_z;

                                if (z < z_pixel){ 
                                    z_pixel = z; 
                                    
                                    float n_x0 = transformed_normals[i3 + 0];
                                    float n_y0 = transformed_normals[i3 + 1];
                                    float n_z0 = transformed_normals[i3 + 2];
                                    
                                    float n_x1 = transformed_normals[i4 + 0];
                                    float n_y1 = transformed_normals[i4 + 1];
                                    float n_z1 = transformed_normals[i4 + 2];
                                    
                                    float n_x2 = transformed_normals[i5 + 0];
                                    float n_y2 = transformed_normals[i5 + 1];
                                    float n_z2 = transformed_normals[i5 + 2];
                                    

                                    float u_x0 = frac_2 * uvs[i6 + 0] + (1.0 - frac_2) * uvs[i7 + 0];
                                    float u_y0 = frac_2 * uvs[i6 + 1] + (1.0 - frac_2) * uvs[i7 + 1];

                                    float u_x1 = frac_2 * uvs[i7 + 0] + (1.0 - frac_2) * uvs[i8 + 0];
                                    float u_y1 = frac_2 * uvs[i7 + 1] + (1.0 - frac_2) * uvs[i8 + 1];
                                                                
                                    float u_x2 = frac_2 * uvs[i8 + 0] + (1.0 - frac_2) * uvs[i6 + 0];
                                    float u_y2 = frac_2 * uvs[i8 + 1] + (1.0 - frac_2) * uvs[i6 + 1];
                                    
                                    switch (show_buffer){
                                        case 0:{
                                            if (!(material_flags & RI_MATERIAL_HAS_TEXTURE)){
                                                frame_pixel = (albedo.a << 24) | (0 << 16) | (0 << 8) | 255;
                                            
                                                break;
                                            }

                                            double ux = (w0 * (u_x0 / z0) + w1 * (u_x1 / z1) + w2 * (u_x2 / z2)) / w_over_z;
                                            double uy = (w0 * (u_y0 / z0) + w1 * (u_y1 / z1) + w2 * (u_y2 / z2)) / w_over_z;
                                            
                                            int texture_width = texture_info[texture_index * 3];
                                            int texture_height = texture_info[texture_index * 3 + 1];
                                            int texture_value_offset = texture_info[texture_index * 3 + 2];
                                            
                                            int ix = fmax((int)(ux * texture_width), 0);
                                            int iy = fmax((int)(uy * texture_height), 0);
                                            
                                            int uv_pixel = (iy * texture_width + ix) * 4 + texture_value_offset;

                                            if (uv_pixel >= texture_width * texture_height * 4 + texture_value_offset)break;
                                            
                                            unsigned char r = textures[uv_pixel + 0];
                                            unsigned char g = textures[uv_pixel + 1];
                                            unsigned char b = textures[uv_pixel + 2];
                                            unsigned char a = textures[uv_pixel + 3];
                                            
                                            frame_pixel = (a << 24) | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 1:{
                                            float z = clamppp_float(z_pixel, 0.0f, highest_z);
                                            
                                            float norm_z = z / highest_z;
                                            
                                            unsigned char intensity = (unsigned char)(norm_z * 255.0f);
                                            
                                            frame_pixel = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;
                                            
                                            break;}
                                        case 2:{
                                            float nx = (w0 * (n_x0 / z0) + w1 * (n_x1 / z1) + w2 * (n_x2 / z2)) / w_over_z;
                                            float ny = (w0 * (n_y0 / z0) + w1 * (n_y1 / z1) + w2 * (n_y2 / z2)) / w_over_z;
                                            float nz = (w0 * (n_z0 / z0) + w1 * (n_z1 / z1) + w2 * (n_z2 / z2)) / w_over_z;
                                            
                                            nx = clamppp_float((nx * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            ny = clamppp_float((ny * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            nz = clamppp_float((nz * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            
                                            unsigned char r = (unsigned char)nx;
                                            unsigned char g = (unsigned char)ny;
                                            unsigned char b = (unsigned char)nz;
                                            
                                            if (!has_normals){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 3:{
                                            float ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;
                                            float uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;
                                            
                                            unsigned char r = (unsigned char)clamppp_float(ux * 255.0f, 0.0f, 255.0f);
                                            unsigned char g = (unsigned char)clamppp_float(uy * 255.0f, 0.0f, 255.0f);
                                            unsigned char b = 0;
                                            
                                            if (!has_uvs){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        default:{
                                            frame_pixel = 0xFF00FFFF;
                                            
                                            break;}
                                    }
                                } 
                            }
                        }
                    }
                    
                    int pixel_coord = (ri_height * 0.5 - id_y) * ri_width + id_x + ri_width * 0.5;
                    
                    if (pixel_coord >= ri_width * ri_height || pixel_coord < 0){
                        continue;
                    }
                    
                    frame_buffer[pixel_coord] = frame_pixel; 
                    }
                }

                
            }
        else if (be_master_renderer){
            if (object_count > 0) {
                erchk(clEnqueueWriteBuffer(queue, object_memory_buffer, CL_TRUE, 0, sizeof(Object) * object_count, objects, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Objects Buffer");
            }

            erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * ri_width * ri_height, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Cleared Frame Buffer");

            erchk(clSetKernelArg(compiled_kernel_transformer, 0, sizeof(cl_mem), &object_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 1, sizeof(cl_mem), &verticies_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 2, sizeof(cl_mem), &normals_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 3, sizeof(cl_mem), &triangles_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 4, sizeof(cl_mem), &transformed_verticies_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 5, sizeof(cl_mem), &transformed_normals_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 6, sizeof(float), (void*)&fov)); 
            erchk(clSetKernelArg(compiled_kernel_transformer, 7, sizeof(int), (void*)&ri_width));
            erchk(clSetKernelArg(compiled_kernel_transformer, 8, sizeof(int), (void*)&ri_height));
            erchk(clSetKernelArg(compiled_kernel_transformer, 9, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_transformer, 10, sizeof(int), (void*)&ri_h_width));
            erchk(clSetKernelArg(compiled_kernel_transformer, 11, sizeof(int), (void*)&ri_h_height));

            size_t size_1d[1] = {object_count};            
 
            erchk(clEnqueueNDRangeKernel(queue, compiled_kernel_transformer, 1, NULL, size_1d, NULL, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(RI_DEBUG_HIGH, "Ran Transformation Kernel");

            erchk(clSetKernelArg(compiled_kernel_master, 0, sizeof(cl_mem), &object_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 1, sizeof(cl_mem), &transformed_verticies_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 2, sizeof(cl_mem), &transformed_normals_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 3, sizeof(cl_mem), &uvs_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 4, sizeof(cl_mem), &triangles_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 5, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 6, sizeof(cl_mem), &textures_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 7, sizeof(cl_mem), &texture_info_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 8, sizeof(int), (void*)&object_count));
            erchk(clSetKernelArg(compiled_kernel_master, 9, sizeof(int), (void*)&ri_width));
            erchk(clSetKernelArg(compiled_kernel_master, 10, sizeof(int), (void*)&ri_height));
            erchk(clSetKernelArg(compiled_kernel_master, 11, sizeof(int), (void*)&show_buffer)); 
            erchk(clSetKernelArg(compiled_kernel_master, 12, sizeof(int), (void*)&frame)); 
            erchk(clSetKernelArg(compiled_kernel_master, 13, sizeof(float), (void*)&fov)); 
            erchk(clSetKernelArg(compiled_kernel_master, 14, sizeof(int), (void*)&face_count)); 
            erchk(clSetKernelArg(compiled_kernel_master, 15, sizeof(int), (void*)&vertex_count)); 
            erchk(clSetKernelArg(compiled_kernel_master, 16, sizeof(int), (void*)&ri_h_width));
            erchk(clSetKernelArg(compiled_kernel_master, 17, sizeof(int), (void*)&ri_h_height));
            erchk(clSetKernelArg(compiled_kernel_master, 18, sizeof(float), (void*)&wireframe_width));
            
            // size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};
            size_t local_size_2d[2] = {16, 16};

            erchk(clEnqueueNDRangeKernel(queue, compiled_kernel_master, 2, NULL, size_2d, NULL, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(RI_DEBUG_HIGH, "Ran Rasterization Kernel");

            erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * ri_width * ri_height, frame_buffer, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Read Frame Buffer");

            frame_buffer[sizeof(Object)] = 0xFFFF00FF;
        }
        else{
            if (polygons == NULL)
            {
                debug_tick_func(0, "Polygons is not Allocated");
                return RI_ERROR;
            }

            if (frame_buffer == NULL)
            {
                debug_tick_func(0, "Frame Buffer is not Allocated");
                return RI_ERROR;
            }
            
            if (show_buffer == RI_BUFFER_Z){
                for (int i_polygon = 2; i_polygon < polygon_count * 9; i_polygon+=3){
                    if (polygons[i_polygon] > highest_z){
                        highest_z = polygons[i_polygon];
                    }
                }
                
                debug_tick_func(1, "Highest Z: %f", highest_z);
            }

            erchk(clSetKernelArg(compiled_kernel_non_master, 0, sizeof(cl_mem), &input_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_non_master, 1, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_non_master, 2, sizeof(int), (void*)&polygon_count));
            erchk(clSetKernelArg(compiled_kernel_non_master, 3, sizeof(int), (void*)&ri_width));
            erchk(clSetKernelArg(compiled_kernel_non_master, 4, sizeof(int), (void*)&ri_height));
            erchk(clSetKernelArg(compiled_kernel_non_master, 5, sizeof(int), (void*)&show_buffer)); 
            erchk(clSetKernelArg(compiled_kernel_non_master, 6, sizeof(float), (void*)&highest_z));
            erchk(clSetKernelArg(compiled_kernel_non_master, 7, sizeof(int), (void*)&ri_h_width));
            erchk(clSetKernelArg(compiled_kernel_non_master, 8, sizeof(int), (void*)&ri_h_height));

            erchk(clEnqueueWriteBuffer(queue, input_memory_buffer, CL_TRUE, 0, sizeof(float) * 3 * 3 * polygon_count, polygons, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(1, "Wrote Polygon Buffer");

            erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * ri_width * ri_height, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(1, "Cleared Frame Buffer");

            size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};

            erchk(clEnqueueNDRangeKernel(queue, compiled_kernel_non_master, 2, NULL, size_2d, local_size_2d, 0, NULL, NULL));
        
            erchk(clFinish(queue));

            erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * ri_width * ri_height, frame_buffer, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Read Frame Buffer");
        }

        if (handle_events){
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    running = 0;
                }
            }
        }

        SDL_UpdateTexture(texture, NULL, frame_buffer, ri_width * sizeof(RI_uint));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        int total_text_height = 0;

        if (show_fps){
            char fps_string[50];
            
            sprintf(fps_string, "%.0f FPS", fps);

            text_surface = TTF_RenderText_Blended(font, fps_string, font_color);
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            text_rect.x = 5;
            text_rect.y = 0;
            text_rect.h = text_surface->h;
            text_rect.w = text_surface->w;

            total_text_height += text_surface->h;

            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

            SDL_FreeSurface(text_surface);
            SDL_DestroyTexture(text_texture);
        }

        if (debug_frame){
            debug_tick_func(0, "Frame: %d", frame);
        }
        
        if (show_frame){
            char frame_string[50];
            
            sprintf(frame_string, "Frame #%d", frame);

            text_surface = TTF_RenderText_Blended(font, frame_string, font_color);
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            text_rect.x = 5;
            text_rect.y = total_text_height;
            text_rect.h = text_surface->h;
            text_rect.w = text_surface->w;

            total_text_height += text_surface->h;

            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

            SDL_FreeSurface(text_surface);
            SDL_DestroyTexture(text_texture);
        }

        if (show_info){
            char frame_string[256];
            
            sprintf(frame_string, "%d objects, %d triangles, %d verticies, %d normals, %d UVS, %d pixels (%dx%d), FPS cap: %d", object_count, face_count, vertex_count, normal_count, uv_count, ri_width * ri_height, ri_width, ri_height, fps_cap);

            text_surface = TTF_RenderText_Blended_Wrapped(font, frame_string, font_color, ri_width);
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            text_rect.x = 5;
            text_rect.y = total_text_height;
            text_rect.h = text_surface->h;
            text_rect.w = text_surface->w;

            total_text_height += text_surface->h;

            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

            SDL_FreeSurface(text_surface);
            SDL_DestroyTexture(text_texture);
        }

        SDL_RenderPresent(renderer);

        frame++;
        
        if (fps_cap > 0 && fps > fps_cap){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            
            double target_frame_time = 1.0 / fps_cap;

            Uint32 delay_time = (Uint32)((target_frame_time - delta_time) * 1000.0);
                
            if (delay_time > 1000){
                debug_tick_func(RI_DEBUG_MEDIUM, "SDL_Delay is over 1000ms");
            }
            else{
                SDL_Delay(delay_time);
            }
        }
        
        if (show_fps || debug_fps || fps_cap > 0){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            fps = 1.0 / delta_time;
        }
        
        if (debug_fps){
            debug(0, "FPS: %lf (%d polygons, %d pixels)", fps, polygon_count, ri_width * ri_height);
        }

        debug_tick_func(1, "Ticked");
        
        return RI_SUCCESS;
    }
    else
    {
        return RI_ERROR;
    }
}

RI_result RI_Stop(int quit){
    debug(RI_DEBUG_HIGH, "Called RI_Stop");

    debug(RI_DEBUG_LOW, "Stopping...");

    running = 0;

    if (!use_cpu){
        clReleaseMemObject(input_memory_buffer);
        clReleaseMemObject(output_memory_buffer);
        clReleaseMemObject(object_memory_buffer);
        clReleaseMemObject(verticies_memory_buffer);
        clReleaseMemObject(transformed_verticies_memory_buffer);
        clReleaseMemObject(normals_memory_buffer);
        clReleaseMemObject(transformed_normals_memory_buffer);
        clReleaseMemObject(uvs_memory_buffer);
        clReleaseMemObject(triangles_memory_buffer);
        clReleaseMemObject(textures_memory_buffer);
        clReleaseMemObject(texture_info_memory_buffer);
        clReleaseKernel(compiled_kernel_non_master);
        clReleaseProgram(kernel_program_non_master);
        clReleaseKernel(compiled_kernel_master);
        clReleaseProgram(kernel_program_master);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
    }

    TTF_CloseFont(font);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    TTF_Quit();

    if (polygons != NULL)
        free(polygons);
    else
        debug(RI_DEBUG_LOW, "Polygons Was Unset on Stop");

    if (frame_buffer != NULL)
        free(frame_buffer);
    else
        debug(RI_DEBUG_LOW, "Frame-Buffer Was Unset on Stop");

    if (objects != NULL)
        free(objects);
    else
        debug(RI_DEBUG_LOW, "Objects Was Unset on Stop");

    if (verticies != NULL)
        free(verticies);
    else
        debug(RI_DEBUG_LOW, "Verticies Was Unset on Stop");

    if (normals != NULL)
        free(normals);
    else
        debug(RI_DEBUG_LOW, "Normals Was Unset on Stop");

    if (uvs != NULL)
        free(uvs);
    else
        debug(RI_DEBUG_LOW, "UVS Was Unset on Stop");

    if (textures != NULL)
        free(textures);
    else
        debug(RI_DEBUG_LOW, "Textures Was Unset on Stop");

    if (texture_info != NULL)
        free(texture_info);
    else
        debug(RI_DEBUG_LOW, "Texture Info Was Unset on Stop");


    debug(RI_DEBUG_LOW, "Stopped");

    if (quit){
        exit(0);
    }

    return RI_SUCCESS;
}
// ----- Renderer Action Functions

// ----- INIT
RI_result Rendering_init(char *title){
    debug(RI_DEBUG_HIGH, "Called Rendering_init");

    debug(RI_DEBUG_LOW, "Initializing Rendering...");

    frame_buffer = malloc(sizeof(RI_uint) * ri_width * ri_height);

    if (frame_buffer == NULL)
    {
        debug(RI_DEBUG_LOW, "Couldn't Allocate Frame Buffer");
        return RI_ERROR;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        debug(RI_DEBUG_LOW, "SDL_Init Failed: %s", SDL_GetError());
        return RI_ERROR;
    }

    if (TTF_Init() == -1) {
        debug(RI_DEBUG_LOW, "TFF_Init Failed: %s", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    if (ri_width <= 0 || ri_height <= 0)
    {
        debug(RI_DEBUG_LOW, "Invalid width or height");
        return RI_ERROR;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ri_width, ri_height, SDL_WINDOW_OPENGL);
    if (!window)
    {
        debug(RI_DEBUG_LOW, "SDL_CreateWindow Failed");
        return RI_ERROR;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        debug(RI_DEBUG_LOW, "SDL_CreateRenderer Failed");
        return RI_ERROR;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ri_width, ri_height);
    if (!texture)
    {
        debug(RI_DEBUG_LOW, "SDL_CreateTexture Failed");
        return RI_ERROR;
    }
    
    font = TTF_OpenFont(font_file, font_size);

    debug(RI_DEBUG_LOW, "Initialized Rendering");

    return RI_SUCCESS;
}

char *load_kernel_source(const char *filename) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    return source;
}


RI_result OpenCL_init(){
    debug(RI_DEBUG_HIGH, "Called OpenCL_init");

    debug(RI_DEBUG_LOW, "Initialiing OpenCL...");

    error = clGetPlatformIDs(1, &platform, &number_of_platforms);
    if (error != CL_SUCCESS || number_of_platforms == 0) {
        debug(RI_DEBUG_LOW, "No OpenCL Platforms (Error %d). Switching to CPU", error);
        
        use_cpu = 1;
        return RI_ERROR;
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);

    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);
    if (error != CL_SUCCESS || number_of_devices == 0) {
        debug(RI_DEBUG_LOW, "No Valid GPUs Found (Error %d). Switching to CPU", error);
        
        use_cpu = 1;
        return RI_ERROR;
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    erchk(error);
    queue = clCreateCommandQueue(context, device, 0, &error);
    erchk(error);

    output_memory_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(RI_uint) * ri_width * ri_height, NULL, &error);
    erchk(error);

    const char *kernel_source_non_master = load_kernel_source("/home/iver/Documents/C-rasterizer/src/RasterIver/kernels/non_master_kernel.cl");

    kernel_program_non_master = clCreateProgramWithSource(context, 1, &kernel_source_non_master, NULL, &error);
    erchk(error);

    debug(1, "Building Non Master Kernel");

    error = clBuildProgram(kernel_program_non_master, 1, &device, NULL, NULL, NULL);
    if (error == -11){
        size_t log_size;
        clGetProgramBuildInfo(kernel_program_non_master, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        
        char *log = malloc(log_size);
        clGetProgramBuildInfo(kernel_program_non_master, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        
        fprintf(stderr, "Build log:\n%s\n", log);
        free(log);            

        RI_Stop(0);
    }
    compiled_kernel_non_master = clCreateKernel(kernel_program_non_master, "raster_kernel", &error);
    erchk(error);

    const char *kernel_source_master = load_kernel_source("/home/iver/Documents/C-rasterizer/src/RasterIver/kernels/master_kernel.cl");

    kernel_program_master = clCreateProgramWithSource(context, 1, &kernel_source_master, NULL, &error);
    erchk(error);

    debug(1, "Building Master Kernel");

    error = clBuildProgram(kernel_program_master, 1, &device, NULL, NULL, NULL);
    if (error == -11){
        size_t log_size;
        clGetProgramBuildInfo(kernel_program_master, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        
        char *log = malloc(log_size);
        clGetProgramBuildInfo(kernel_program_master, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        
        fprintf(stderr, "Build log:\n%s\n", log);
        free(log);            

        RI_Stop(0);
    }

    compiled_kernel_master = clCreateKernel(kernel_program_master, "raster_kernel", &error);
    erchk(error);

    erchk(clGetKernelWorkGroupInfo(compiled_kernel_master, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL));

    debug(RI_DEBUG_MEDIUM, "Local Size: %d", local_size);


    const char *kernel_source_transformer = load_kernel_source("/home/iver/Documents/C-rasterizer/src/RasterIver/kernels/transformer.cl");

    kernel_program_transformer = clCreateProgramWithSource(context, 1, &kernel_source_transformer, NULL, &error);
    erchk(error);

    debug(1, "Building Transformer Kernel");

    error = clBuildProgram(kernel_program_transformer, 1, &device, NULL, NULL, NULL);
    if (error == -11){
        size_t log_size;
        clGetProgramBuildInfo(kernel_program_transformer, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        
        char *log = malloc(log_size);
        clGetProgramBuildInfo(kernel_program_transformer, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        
        fprintf(stderr, "Build log:\n%s\n", log);
        free(log);            

        RI_Stop(0);
    }

    compiled_kernel_transformer = clCreateKernel(kernel_program_transformer, "transformer_kernel", &error);
    erchk(error);

    size_2d[0] = ri_width;
    size_2d[1] = ri_height;

    pattern = 0x22222222;

    debug(RI_DEBUG_LOW, "Initialized OpenCL");

    return RI_SUCCESS;
}

RI_result RI_Init(int RI_WindowWidth, int RI_WindowHeight, char *RI_WindowTitle){
    debug(RI_DEBUG_HIGH, "Called RI_Init");

    srand(time(NULL));                                                         

    ri_width = RI_WindowWidth;
    ri_height = RI_WindowHeight;

    ri_h_width = RI_WindowWidth * 0.5;
    ri_h_height = RI_WindowHeight * 0.5;

    if (!use_cpu && OpenCL_init() == RI_ERROR){
        if (!use_cpu){
            return RI_ERROR;
        }
    }

    if (Rendering_init(RI_WindowTitle) == RI_ERROR){
        return RI_ERROR;
    }

    return RI_SUCCESS;
}
// ----- INIT