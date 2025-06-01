#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>
#include "../headers/rasteriver.h"
#include <stdarg.h>
#include <stdio.h>
#include <SDL2/SDL_ttf.h>

// ----- Internal Variables
int width;
int height;

float highest_z = 0;

int polygon_count;
RI_polygons polygons = NULL;

int object_count;
RI_objects objects;
RI_verticies verticies;
RI_verticies normals;
RI_verticies uvs;
RI_triangles triangles;
RI_textures textures;

int running = 1;
int frame = 0;

int show_buffer = 0;
int show_debug = 0;
int debug_verbose = 0;
int show_fps = 0;
int debug_fps = 0;
int clean_polygons = 0;
int populate_polygons = 0;
int be_master_renderer = 0;
int debug_frame = 0;
int show_frame = 0;
int show_info = 0;
int debug_tick = 0;

Uint64 start_time;
double frame_time_ms;
double fps;
double elapsed_ticks;
double delta_time;
int fps_cap = -1;
// ----- Internal Variables

// ----- Rendering Vars
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

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
cl_mem normals_memory_buffer;
cl_mem uvs_memory_buffer;

cl_program kernel_program_non_master;
cl_kernel compiled_kernel_non_master;

cl_program kernel_program_master;
cl_kernel compiled_kernel_master;

size_t size_2d[2];
size_t local_size;

RI_uint pattern;
// ----- OpenCL Vars

// ----- Internal Functions
RI_result debug(int verbose, char *string, ...)
{
    if (!show_debug || (verbose && !debug_verbose)){
        return RI_ERROR;
    }

    va_list args;
    va_start(args, string);

    char prefix[100] = "[RasterIver] ";

    strcat(prefix, string);

    vprintf(prefix, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

RI_result debug_tick_func(int verbose, char *string, ...)
{
    if (!show_debug || (verbose && !debug_verbose) || !debug_tick){
        return RI_ERROR;
    }

    va_list args;
    va_start(args, string);

    char prefix[100] = "[RasterIver] ";

    strcat(prefix, string);

    vprintf(prefix, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

RI_result erchk_func(cl_int error, int line, char *file)
{
    if (error != CL_SUCCESS)
    {
        debug(0, "OpenCL Error: %d at line %d at file %s", error, line, file);

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
RI_result RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value)
{
    switch (RI_FlagToSet)
    {
    case RI_FLAG_DEBUG:
        show_debug = RI_Value;
        break;

    case RI_FLAG_DEBUG_VERBOSE:
        debug_verbose = RI_Value;
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
        
    default:
        return RI_INVALID_FLAG;
    }

    return RI_SUCCESS;
}

RI_result RI_SetBackground(RI_uint RI_BackgroundColor)
{
    pattern = RI_BackgroundColor;

    return RI_SUCCESS;
}

RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest){
    polygon_count = RI_PolygonsToRequest;
    
    int size = sizeof(float) * 3 * 3 * polygon_count;
    
    debug(1, "Requesting %d Polygons... (%d bytes)", polygon_count, size);
    
    if (polygons != NULL)
    {
        free(polygons);
    }
    
    polygons = malloc(size);
    
    if (polygons == NULL)
    {
        debug(0, "Malloc Error");
        return (float*)RI_ERROR;
    }
    
    for (int p = 0; p < polygon_count * 9; p += 3){     
        if (clean_polygons){
            polygons[p] = INFINITY;
            polygons[p + 1] = INFINITY;
            polygons[p + 2] = INFINITY;
        }
        else if (populate_polygons){
            polygons[p] = rand() % width;
            polygons[p + 1] = rand() % height;
            polygons[p + 2] = rand() % ((width + height) / 2);
        }
    }
    
    input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, polygons, &error);
    
    if (input_memory_buffer == NULL)
    {
        debug(0, "clCreateBuffer Failed for Requested Polygons");
    }

    erchk(error);
    
    debug(1, "Request for %d Polygons Granted", polygon_count);
    
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
    strncpy(result, string + start, end - start);
}

void malloc_objects(int objects, char **file_names){
    for (int i = 0; i < objects; i++){
        FILE *file = fopen(file_names[i], "r");

        if(file == NULL){
            debug(0, "Error Opening Object File");
            RI_Stop(0);
        }

        char line[256];

        while (fgets(line, sizeof(line), file)) {
            if (line[0] == 'f' && line[1] == ' ') {
                face_count++;
            } 
            else if (line[0] == 'v' && line[1] == ' ') {
                vertex_count++;
            } 
            else if (line[0] == 'v' && line[1] == 'n') {
                normal_count++;
            } 
            else if (line[0] == 'v' && line[1] == 't') {
                uv_count++;
            } 
        }

        fclose(file);
    }

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

    return;
}

load_object_return load_object(char *object_path, int object_offset, int base){
    debug(1, "Loading Object #%d...", object_offset + 1);

    debug(1, "Opening File \"%s\"...", object_path);

    FILE *file = fopen(object_path, "r");

    if(file == NULL){
        debug(0, "Error Opening Object File");
        RI_Stop(0);
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
                &triangles[(ct + loading_object_current_faces_count) * ts + 0], &triangles[(ct + loading_object_current_faces_count) * ts + 3], &triangles[(ct + loading_object_current_faces_count) * ts + 6], 
                &triangles[(ct + loading_object_current_faces_count) * ts + 1], &triangles[(ct + loading_object_current_faces_count) * ts + 4], &triangles[(ct + loading_object_current_faces_count) * ts + 7], 
                &triangles[(ct + loading_object_current_faces_count) * ts + 2], &triangles[(ct + loading_object_current_faces_count) * ts + 5], &triangles[(ct + loading_object_current_faces_count) * ts + 8]);

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
                        &triangles[(ct + loading_object_current_faces_count) * ts + 0], &triangles[(ct + loading_object_current_faces_count) * ts + 6], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 1], &triangles[(ct + loading_object_current_faces_count) * ts + 7], 
                        &triangles[(ct + loading_object_current_faces_count) * ts + 2], &triangles[(ct + loading_object_current_faces_count) * ts + 8]);
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
            debug(1, "OBJ file uses X/X/X X/X/X X/X/X");
            break;
            
        case 1:
            debug(1, "OBJ file uses X//X X//X X//X");
            break;

        case 2:
            debug(1, "OBJ file uses X X X");
            break;
        
        default:
            break;
    }

    debug(1, "%d Triangles", ct);
    debug(1, "%d Verticies", cv);
    debug(1, "%d Normals", cn);
    debug(1, "%d UVS", cu);

    objects[base + 9] = ct; // triangle count

    loading_object_current_faces_count += ct;
    loading_object_current_verticies_count += cv;
    loading_object_current_normals_count += cn;
    loading_object_current_uvs_count += cu;

    fclose(file);

    load_object_return return_values = {
        0, 0, 0, 0, 0
    };

    return return_values;
}

RI_objects RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest){
    object_count = RI_ObjectsToRequest;
    
    debug(1, "Requesting %d Objects...", object_count);

    if (objects != NULL)
    {
        free(objects);
    }

    int object_arary_size = sizeof(int) * object_size * RI_ObjectsToRequest;

    objects = malloc(object_arary_size);
    
    if (objects == NULL){
        debug(0, "Malloc Error");
    }

    char **file_names = malloc(RI_ObjectsToRequest * sizeof(char *));

    for (int object = 0; object < object_count; object++){
        file_names[object] = RI_ObjectBuffer[object].file_path;
    }

    malloc_objects(RI_ObjectsToRequest, file_names);
    
    free(file_names);

    loading_object_current_verticies_count = 0;
    loading_object_current_normals_count = 0;
    loading_object_current_uvs_count = 0;
    loading_object_current_faces_count = 0;

    for (int object = 0; object < object_count; object++){
        RI_newObject loading_object_current_object_count = RI_ObjectBuffer[object];

        int base = object * object_size;
        objects[base + 10] = loading_object_current_faces_count; // triangle offset
        objects[base + 11] = loading_object_current_verticies_count; // vertex offset
        objects[base + 12] = loading_object_current_normals_count; // normal offset
        objects[base + 13] = loading_object_current_uvs_count; // uvs offset

        load_object((char *)loading_object_current_object_count.file_path, object, base);
        
        objects[base + 0] = loading_object_current_object_count.x; // x
        objects[base + 1] = loading_object_current_object_count.y; // y
        objects[base + 2] = loading_object_current_object_count.z; // z
        objects[base + 3] = loading_object_current_object_count.r_x; // rotation x
        objects[base + 4] = loading_object_current_object_count.r_y; // rotation y
        objects[base + 5] = loading_object_current_object_count.r_z; // rotation z
        objects[base + 6] = loading_object_current_object_count.s_x; // scale x
        objects[base + 7] = loading_object_current_object_count.s_y; // scale y
        objects[base + 8] = loading_object_current_object_count.s_z; // scale z

        debug(1, "Triangle Index: %d", objects[base + 10]);
        debug(1, "Vertex Index: %d", objects[base + 11]);
        debug(1, "Normal Index: %d", objects[base + 12]);
        debug(1, "UV Index: %d", objects[base + 13]);
    }
    
    for (int i = 0; i < face_count * 9; i++){
        triangles[i]--;
    }

    debug(1, "Allocated %d Bytes for Objects", sizeof(RI_verticies) * uv_count * vs + sizeof(RI_triangles) * face_count * vs + sizeof(RI_verticies) * vertex_count * vs + sizeof(RI_verticies) * normal_count * vs + object_arary_size);

    object_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, object_arary_size, objects, &error);
    erchk(error);
    
    if (object_memory_buffer == NULL){
        debug(0, "clCreateBuffer Failed for Objects Buffer");
    }

    if (face_count > 0){
        triangles_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_triangles) * face_count * ts, triangles, &error);
        erchk(error);
    
        if (triangles_memory_buffer == NULL){
            debug(0, "clCreateBuffer Failed for Triangles Buffer");
        }
    }

    if (vertex_count > 0){
        verticies_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * vertex_count * vs, verticies, &error);
        erchk(error);
    
        if (verticies_memory_buffer == NULL){
            debug(0, "clCreateBuffer Failed for Verticies Buffer");
        }
    }

    if (normal_count > 0){
        normals_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * normal_count * vs, normals, &error);
        erchk(error);
    
        if (normals_memory_buffer == NULL){
            debug(0, "clCreateBuffer Failed for Normals Buffer");
        }
    }

    if (uv_count > 0){
        uvs_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * uv_count * vs, uvs, &error);
        erchk(error);

        if (uvs_memory_buffer == NULL){
            debug(0, "clCreateBuffer Failed for UVS Buffer");
        }
    }

    debug(1, "Request for %d Objects Granted", object_count);
    
    return objects;
}

RI_result RI_SetFpsCap(int RI_FpsCap){
    fps_cap = RI_FpsCap;

    return RI_SUCCESS;
}

RI_result RI_SetFontColor(RI_uint RI_FontColorRGBA){
    font_color.r = (RI_FontColorRGBA >> 24) & 0xFF;
    font_color.g = (RI_FontColorRGBA >> 16) & 0xFF;
    font_color.b = (RI_FontColorRGBA >> 8) & 0xFF;
    font_color.a = RI_FontColorRGBA & 0xFF;

    return RI_SUCCESS;
}

RI_result RI_SetFontSize(int RI_FontSize){
    font_size = RI_FontSize;
    
    return RI_SUCCESS;    
}

RI_result RI_SetFontFile(char *RI_PathToFontFile){
    TTF_Font* font_check = TTF_OpenFont(RI_PathToFontFile, 24);

    if (font_check == NULL){
        return RI_ERROR;
    }

    font_file = RI_PathToFontFile;

    font = TTF_OpenFont(font_file, font_size);

    return RI_SUCCESS;
}
// ----- Set Value Functions
int is_intersecting(float a, float b, float c, float d, float p, float q, float r, float s) { \
    float det, gamma, lambda; \
    \
    det = (c - a) * (s - q) - (r - p) * (d - b); \
    \
    if (det == 0) { \
        return 1; \
    }  \
    else { \
        lambda = ((s - q) * (r - a) + (p - r) * (s - b)) / det; \
        gamma = ((b - d) * (r - a) + (c - a) * (s - b)) / det; \
        return (0 < lambda && lambda < 1) && (0 < gamma && gamma < 1); \
    } \
} \
// ----- Renderer Action Functions
RI_result RI_Tick(){
    if (show_fps || debug_fps){
        start_time = SDL_GetPerformanceCounter();
    }
    
    debug_tick_func(1, "Ticking...");
    
    if (running)
    {
        if (frame_buffer == NULL)
        {
            debug_tick_func(0, "Frame Buffer is not Allocated");
            return RI_ERROR;
        }

//     for (int id_y = 0; id_y < height; id_y++){
//         debug(0, "%d", id_y);
//         for (int id_x = 0; id_x < width; id_x++){
//     \
//     float z_pixel = 0; \
//     Uint32 frame_pixel = 0x22222222; \
//     \
//     float biggest_z = 0;\
//     \
//     for (int object = 0; object < object_count; object++){ \
//         int base = object * 15;\
//         \
//         int object_x =   objects[base + 0]; \
//         int object_y =   objects[base + 1]; \
//         int object_z =   objects[base + 2]; \
//         int object_r_x = objects[base + 3]; \
//         int object_r_y = objects[base + 4]; \
//         int object_r_z = objects[base + 5]; \
//         int object_s_x = objects[base + 6]; \
//         int object_s_y = objects[base + 7]; \
//         int object_s_z = objects[base + 8]; \
//         \
//         int triangle_count = objects[base + 9];\
//         int triangle_index = objects[base + 10];\
//         int vertex_index = objects[base + 11];\
//         int texture_index = objects[base + 14];\
//         \
//         for (int triangle = 0; triangle < triangle_count; triangle++){\
//             int triangle_base = (triangle + triangle_index) * 9; \
// \
//             int i0 = (vertex_index + triangles[triangle_base + 0]) * 3;\
//             int i1 = (vertex_index + triangles[triangle_base + 1]) * 3;\
//             int i2 = (vertex_index + triangles[triangle_base + 2]) * 3;\
//             \
//             float x0 = verticies[i0 + 0] * object_s_x + object_x;\
//             float y0 = verticies[i0 + 1] * object_s_y + object_y;\
//             float z0 = verticies[i0 + 2] * object_s_z + object_z;\
//             \
//             float x1 = verticies[i1 + 0] * object_s_x + object_x;\
//             float y1 = verticies[i1 + 1] * object_s_y + object_y;\
//             float z1 = verticies[i1 + 2] * object_s_z + object_z;\
//             \
//             float x2 = verticies[i2 + 0] * object_s_x + object_x;\
//             float y2 = verticies[i2 + 1] * object_s_y + object_y;\
//             float z2 = verticies[i2 + 2] * object_s_z + object_z;\
//             \
//             if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){\
//                 continue;\
//             }\
//             \
//             float smallest_x = x0; \
//             float largest_x = x0; \
//             float smallest_y = y0; \
//             float largest_y = y0; \
//             \
//             if (x0 > largest_x) largest_x = x0;\
//             if (x1 > largest_x) largest_x = x1;\
//             if (x2 > largest_x) largest_x = x2;\
//             \
//             if (x0 < smallest_x) smallest_x = x0;\
//             if (x1 < smallest_x) smallest_x = x1;\
//             if (x2 < smallest_x) smallest_x = x2;\
//             \
//             if (y0 > largest_y) largest_y = y0;\
//             if (y1 > largest_y) largest_y = y1;\
//             if (y2 > largest_y) largest_y = y2;\
//             \
//             if (y0 < smallest_y) smallest_y = y0;\
//             if (y1 < smallest_y) smallest_y = y1;\
//             if (y2 < smallest_y) smallest_y = y2;\
//             \
//             smallest_x = fmin(smallest_x, 0); \
//             largest_x = fmax(largest_x, width); \
//             smallest_y = fmin(smallest_y, 0); \
//             largest_y = fmax(largest_y, height); \
//             \
//             if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ \
//                 int intersections = 0; \
//                 \
//                 intersections += is_intersecting(id_x, id_y, 10000, 100000, x0, y0, x1, y1); \
//                 intersections += is_intersecting(id_x, id_y, 10000, 100000, x1, y1, x2, y2); \
//                 intersections += is_intersecting(id_x, id_y, 10000, 100000, x2, y2, x0, y0); \
//                 \
//                 if (intersections % 2 == 0){ \
//                     continue; \
//                 } \
//                 \
//                 float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); \
//                 float w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; \
//                 float w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y2)) / denominator; \
//                 float w2 = 1.0 - w0 - w1; \
//                 \
//                 if (denominator < 0) { \
//                     w0 = -w0; \
//                     w1 = -w1; \
//                     w2 = -w2; \
//                     denominator = -denominator; \
//                 } \
//                 \
//                 float z = w0 * z0 + w1 * z1 + w2 * z2; \
//                 \
//                 if (z < 0){ \
//                     z *= -1; \
//                 } \
//                 \
//                 if (z > z_pixel){ \
//                     z_pixel = z; \
//                 } \
//                 \
//                 else { \
//                     continue; \
//                 } \
// \
//                 frame_pixel = 0xFFFFFFFF / triangle_count * (triangle + 1); \
//             } \
//         }\
//     }\
//     \
//     frame_buffer[id_y * width + id_x] = frame_pixel; \
//         }}

        if (be_master_renderer){
            erchk(clSetKernelArg(compiled_kernel_master, 0, sizeof(cl_mem), &object_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 1, sizeof(cl_mem), &verticies_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 2, sizeof(cl_mem), &normals_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 3, sizeof(cl_mem), &uvs_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 4, sizeof(cl_mem), &triangles_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 5, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 6, sizeof(int), (void*)&object_count));
            erchk(clSetKernelArg(compiled_kernel_master, 7, sizeof(int), (void*)&width));
            erchk(clSetKernelArg(compiled_kernel_master, 8, sizeof(int), (void*)&height));
            erchk(clSetKernelArg(compiled_kernel_master, 9, sizeof(int), (void*)&show_buffer)); 

            if (object_count > 0) {
                erchk(clEnqueueWriteBuffer(queue, object_memory_buffer, CL_TRUE, 0, sizeof(int) * object_size * object_count, objects, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Objects Buffer");
            }

            if (vertex_count > 0){
                erchk(clEnqueueWriteBuffer(queue, verticies_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * vertex_count, verticies, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Verticies Buffer");
            }

            if (normal_count > 0){
                erchk(clEnqueueWriteBuffer(queue, normals_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * normal_count, normals, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Normals Buffer");
            }

            if (uv_count > 0){
                erchk(clEnqueueWriteBuffer(queue, uvs_memory_buffer, CL_TRUE, 0, sizeof(float) * vs * uv_count, uvs, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote UVS Buffer");
            }

            if (face_count > 0){
                erchk(clEnqueueWriteBuffer(queue, triangles_memory_buffer, CL_TRUE, 0, sizeof(int) * ts * face_count, triangles, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Triangles Buffer");
            }
            
            erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * width * height, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Cleared Frame Buffer");

            size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};

            // for (int i = 0; i < passes; i++)
            // {
            erchk(clEnqueueNDRangeKernel(queue, compiled_kernel_master, 2, NULL, size_2d, local_size_2d, 0, NULL, NULL));
            erchk(clFinish(queue));

            erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * width * height, frame_buffer, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Read Frame Buffer");
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
                for (int p = 2; p < polygon_count * 9; p+=3){
                    if (polygons[p] > highest_z){
                        highest_z = polygons[p];
                    }
                }
                
                debug_tick_func(1, "Highest Z: %f", highest_z);
            }

            erchk(clSetKernelArg(compiled_kernel_non_master, 0, sizeof(cl_mem), &input_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_non_master, 1, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_non_master, 2, sizeof(int), (void*)&polygon_count));
            erchk(clSetKernelArg(compiled_kernel_non_master, 3, sizeof(int), (void*)&width));
            erchk(clSetKernelArg(compiled_kernel_non_master, 4, sizeof(int), (void*)&height));
            erchk(clSetKernelArg(compiled_kernel_non_master, 5, sizeof(int), (void*)&show_buffer)); 
            erchk(clSetKernelArg(compiled_kernel_non_master, 6, sizeof(float), (void*)&highest_z));

            erchk(clEnqueueWriteBuffer(queue, input_memory_buffer, CL_TRUE, 0, sizeof(float) * 3 * 3 * polygon_count, polygons, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(1, "Wrote Polygon Buffer");

            erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * width * height, 0, NULL, NULL));
            erchk(clFinish(queue));

            debug_tick_func(1, "Cleared Frame Buffer");

            size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};

            // for (int i = 0; i < passes; i++)
            // {
            erchk(clEnqueueNDRangeKernel(queue, compiled_kernel_non_master, 2, NULL, size_2d, local_size_2d, 0, NULL, NULL));
        
            erchk(clFinish(queue));

            erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * width * height, frame_buffer, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Read Frame Buffer");
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
            }
        }

        SDL_UpdateTexture(texture, NULL, frame_buffer, width * sizeof(RI_uint));

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
        }

        if (show_info){
            char frame_string[256];
            
            sprintf(frame_string, "%d objects, %d triangles, %d verticies, %d normals, %d UVS, %d pixels (%dx%d), FPS cap: %d", object_count, face_count, vertex_count, normal_count, uv_count, width * height, width, height, fps_cap);

            text_surface = TTF_RenderText_Blended_Wrapped(font, frame_string, font_color, width);
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            text_rect.x = 5;
            text_rect.y = total_text_height;
            text_rect.h = text_surface->h;
            text_rect.w = text_surface->w;

            total_text_height += text_surface->h;

            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        }

        SDL_RenderPresent(renderer);

        frame++;
        
        if (fps_cap > 0 && fps > fps_cap){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            
            double target_frame_time = 1.0 / fps_cap;
            
            SDL_Delay((Uint32)((target_frame_time - delta_time) * 1000.0));
        }
        
        if (show_fps || debug_fps){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            fps = 1.0 / delta_time;
        }
        
        if (debug_fps){
            debug_tick_func(0, "FPS: %lf (%d polygons, %d pixels)", fps, polygon_count, width * height);
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
    debug(0, "Stopping...");

    running = 0;

    clReleaseMemObject(input_memory_buffer);
    clReleaseMemObject(output_memory_buffer);
    clReleaseMemObject(object_memory_buffer);
    clReleaseMemObject(verticies_memory_buffer);
    clReleaseMemObject(normals_memory_buffer);
    clReleaseMemObject(uvs_memory_buffer);
    clReleaseMemObject(triangles_memory_buffer);
    clReleaseKernel(compiled_kernel_non_master);
    clReleaseProgram(kernel_program_non_master);
    clReleaseKernel(compiled_kernel_master);
    clReleaseProgram(kernel_program_master);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    TTF_Quit();

    if (polygons != NULL)
        free(polygons);
    else
        debug(0, "Polygons Was Unset on Stop");

    if (frame_buffer != NULL)
        free(frame_buffer);
    else
        debug(0, "Frame-Buffer Was Unset on Stop");

    if (objects != NULL)
        free(objects);
    else
        debug(0, "Objects Was Unset on Stop");

    if (verticies != NULL)
        free(verticies);
    else
        debug(0, "Verticies Was Unset on Stop");

    if (normals != NULL)
        free(normals);
    else
        debug(0, "Normals Was Unset on Stop");

    if (uvs != NULL)
        free(uvs);
    else
        debug(0, "UVS Was Unset on Stop");

    debug(0, "Stopped");

    if (quit){
        exit(0);
    }

    return RI_SUCCESS;
}
// ----- Renderer Action Functions

// ----- INIT
RI_result Rendering_init(char *title){
    debug(0, "Initializing Rendering...");

    frame_buffer = malloc(sizeof(RI_uint) * width * height);

    if (frame_buffer == NULL)
    {
        debug(0, "Couldn't Allocate Frame Buffer");
        return RI_ERROR;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        debug(0, "SDL_Init Failed: %s", SDL_GetError());
        return RI_ERROR;
    }

    if (TTF_Init() == -1) {
        debug(0, "TFF_Init Failed: %s", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    if (width <= 0 || height <= 0)
    {
        debug(0, "Invalid width or height");
        return RI_ERROR;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (!window)
    {
        debug(0, "SDL_CreateWindow Failed");
        return RI_ERROR;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        debug(0, "SDL_CreateRenderer Failed");
        return RI_ERROR;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture)
    {
        debug(0, "SDL_CreateTexture Failed");
        return RI_ERROR;
    }
    
    font = TTF_OpenFont(font_file, font_size);

    text_surface = TTF_RenderText_Solid(font, "FPS", font_color);
    if (text_surface == NULL){
        debug(0, "TTF_RenderText_Solid Failed: %s", TTF_GetError());
        return RI_ERROR;
    }

    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (text_texture == NULL){
        debug(0, "SDL_CreateTextureFromSurface Failed");
        return RI_ERROR;
    }

    debug(0, "Initialized Rendering");

    return RI_SUCCESS;
}

RI_result OpenCL_init(){
    debug(0, "Initialiing OpenCL...");

    clGetPlatformIDs(1, &platform, &number_of_platforms);

    if (number_of_platforms == 0)
    {
        debug(0, "No OpenCL Platforms");
        return RI_ERROR;
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);

    if (number_of_devices == 0)
    {
        debug(0, "No Valid GPU's Found");
        return RI_ERROR;
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    erchk(error);
    queue = clCreateCommandQueue(context, device, 0, &error);
    erchk(error);

    output_memory_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(RI_uint) * width * height, NULL, &error);
    erchk(error);

    kernel_program_non_master = clCreateProgramWithSource(context, 1, &kernel_source_non_master, NULL, &error);
    erchk(error);

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


    kernel_program_master = clCreateProgramWithSource(context, 1, &kernel_source_master, NULL, &error);
    erchk(error);

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

    debug(1, "Local Size: %d", local_size);

    size_2d[0] = width;
    size_2d[1] = height;

    pattern = 0x22222222;

    debug(0, "Initialized OpenCL");

    return RI_SUCCESS;
}

RI_result RI_Init(int RI_WindowWidth, int RI_WindowHeight, char *RI_WindowTitle)
{
    srand(time(NULL));                                                         

    width = RI_WindowWidth;
    height = RI_WindowHeight;

    if (OpenCL_init() == RI_ERROR){
        return RI_ERROR;
    }

    if (Rendering_init(RI_WindowTitle) == RI_ERROR){
        return RI_ERROR;
    }

    return RI_SUCCESS;
}
// ----- INIT