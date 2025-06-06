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

int *texture_info;

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
cl_mem textures_memory_buffer;
cl_mem texture_info_memory_buffer;

cl_program kernel_program_non_master;
cl_kernel compiled_kernel_non_master;

cl_program kernel_program_master;
cl_kernel compiled_kernel_master;

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

    char prefix[100] = "[RasterIver] ";

    strcat(prefix, string);

    vprintf(prefix, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

RI_result debug_tick_func(int verbose, char *string, ...)
{
    if (!show_debug || (verbose && debug_level != RI_DEBUG_HIGH) || !debug_tick){
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
        debug(RI_DEBUG_LOW, "OpenCL Error: %d at line %d at file %s", error, line, file);

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

    default:
        return RI_INVALID_FLAG;
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

void malloc_objects(int objects, char **file_names){
    debug(RI_DEBUG_HIGH, "Called malloc_objects");

    for (int i = 0; i < objects; i++){
        debug(RI_DEBUG_HIGH, "file_names[i] is %s", file_names[i]);

        FILE *file = fopen(file_names[i], "r");

        if(file == NULL){
            debug(RI_DEBUG_LOW, "Error Opening Object File");
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

    debug(RI_DEBUG_HIGH, "Left malloc_objects");

    return;
}

load_object_return load_object(char *object_path, int object_offset, int base){
    debug(RI_DEBUG_HIGH, "Called load_object");

    debug(RI_DEBUG_MEDIUM, "Loading Object #%d...", object_offset + 1);

    debug(RI_DEBUG_MEDIUM, "Opening File \"%s\"...", object_path);

    FILE *file = fopen(object_path, "r");

    if(file == NULL){
        debug(RI_DEBUG_LOW, "Error Opening Object File");
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

    objects[base + 9] = ct; // triangle count

    loading_object_current_faces_count += ct;
    loading_object_current_verticies_count += cv;
    loading_object_current_normals_count += cn;
    loading_object_current_uvs_count += cu;

    fclose(file);

    load_object_return return_values = {
        0, 0, 0, 0, 0
    };

    debug(RI_DEBUG_HIGH, "Left load_object");

    return return_values;
}

RI_objects RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest){
    debug(RI_DEBUG_HIGH, "Called RI_RequestObjects");

    object_count = RI_ObjectsToRequest;
    
    debug(RI_DEBUG_MEDIUM, "Requesting %d Objects...", object_count);

    if (objects != NULL)
    {
        free(objects);
    }

    int object_arary_size = sizeof(float) * object_size * RI_ObjectsToRequest;

    objects = malloc(object_arary_size);
    
    if (objects == NULL){
        debug(RI_DEBUG_LOW, "Malloc Error");
    }

    char **file_names = malloc(RI_ObjectsToRequest * sizeof(char *));
    char **texture_names = malloc(RI_ObjectsToRequest * sizeof(char *));

    for (int object = 0; object < object_count; object++){
        file_names[object] = RI_ObjectBuffer[object].file_path;
        debug(RI_DEBUG_HIGH, "file_names[object] is %s", file_names[object]);
        texture_names[object] = "blahblahblah placeholder (this is some salt)";
    }

    malloc_objects(RI_ObjectsToRequest, file_names);
    
    free(file_names);

    loading_object_current_verticies_count = 0;
    loading_object_current_normals_count = 0;
    loading_object_current_uvs_count = 0;
    loading_object_current_faces_count = 0;

    texture_count = 0;
    int is_this_texture_name_already_in_the_texture_names_array = 0;

    textures_size = 0;

    for (int object = 0; object < object_count; object++){
        RI_newObject *loading_object_current_object = &RI_ObjectBuffer[object];
        
        int base = object * object_size;
        objects[base + 10] = loading_object_current_faces_count; // triangle offset
        objects[base + 11] = loading_object_current_verticies_count; // vertex offset
        objects[base + 12] = loading_object_current_normals_count; // normal offset
        objects[base + 13] = loading_object_current_uvs_count; // uvs offset

        is_this_texture_name_already_in_the_texture_names_array = 0;

        for (int object_texture = 0; object_texture < object_count; object_texture++){
            if (strcmp(texture_names[object_texture], loading_object_current_object->texture) == 0){
                debug(RI_DEBUG_HIGH, "Not Reloading Texture \"%s\" (texture #%d) (compared %s to %s)", loading_object_current_object->texture, object_texture, texture_names[object_texture], loading_object_current_object->texture);

                objects[base + 14] = object_texture; // texture offset
                is_this_texture_name_already_in_the_texture_names_array = 1;
                break;
            }
        }    

        if (!is_this_texture_name_already_in_the_texture_names_array){
            texture_names[texture_count] = loading_object_current_object->texture;
            objects[base + 14] = texture_count; // texture offset
            texture_count++;
            int texture_width, texture_height, channels;
            stbi_load(loading_object_current_object->texture, &texture_width, &texture_height, &channels, 4);
            debug(RI_DEBUG_MEDIUM, "Texture (%s) Loaded With Size %dx%d (%d channels)", loading_object_current_object->texture, texture_width, texture_height, channels);
            textures_size += texture_width * texture_height;
        }

        load_object((char *)loading_object_current_object->file_path, object, base);
        
        objects[base + 0] = loading_object_current_object->x; // x
        objects[base + 1] = loading_object_current_object->y; // y
        objects[base + 2] = loading_object_current_object->z; // z
        objects[base + 3] = loading_object_current_object->r_x; // rotation x
        objects[base + 4] = loading_object_current_object->r_y; // rotation y
        objects[base + 5] = loading_object_current_object->r_z; // rotation z
        objects[base + 15] = loading_object_current_object->r_w; // rotation w
        objects[base + 6] = loading_object_current_object->s_x; // scale x
        objects[base + 7] = loading_object_current_object->s_y; // scale y
        objects[base + 8] = loading_object_current_object->s_z; // scale z

        debug(RI_DEBUG_MEDIUM, "Triangle Index: %d", objects[base + 10]);
        debug(RI_DEBUG_MEDIUM, "Vertex Index: %d", objects[base + 11]);
        debug(RI_DEBUG_MEDIUM, "Normal Index: %d", objects[base + 12]);
        debug(RI_DEBUG_MEDIUM, "UV Index: %d", objects[base + 13]);
    }
    
    debug(RI_DEBUG_MEDIUM, "Allocating Texture Buffers");

    textures = malloc(sizeof(unsigned char) * textures_size * 4);
    texture_info = malloc(sizeof(int) * tis * texture_count);

    if (!textures){
        debug(RI_DEBUG_MEDIUM, "Failed to Allocate Texture Buffer");
    }

    if (!texture_info){
        debug(RI_DEBUG_MEDIUM, "Failed to Allocate Texture Info Buffer");
    }

    if (texture_count > 0){
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
    }

    int value_offset = 0;
    for (int i_current_texture = 0; i_current_texture < texture_count; i_current_texture++){
        char *current_texture_name = texture_names[i_current_texture];

        int temp_width, temp_height;
        RI_textures temp_texture = stbi_load(current_texture_name, &temp_width, &temp_height, NULL, 4);

        texture_info[i_current_texture * tis] = temp_width;
        texture_info[i_current_texture * tis + 1] = temp_height;
        texture_info[i_current_texture * tis + 2] = value_offset;

        debug(RI_DEBUG_HIGH, "Texture Info for Texture #%d: width: %d, height: %d, offset: %d", i_current_texture, texture_info[i_current_texture * tis], texture_info[i_current_texture * tis + 1], texture_info[i_current_texture * tis + 2]);

        for (int i_current_value = 0; i_current_value < temp_width * temp_height * 4; i_current_value++){
            textures[i_current_value + value_offset] = temp_texture[i_current_value];
        }

        value_offset += temp_width * temp_height * 4;
    }

    debug(RI_DEBUG_MEDIUM, "%d Textures Totalling %d Bytes", texture_count, sizeof(unsigned char) * textures_size * 4);
    
    for (int i = 0; i < face_count * 9; i++){
        triangles[i]--;
    }

    debug(RI_DEBUG_MEDIUM, "Allocated %d Bytes for Objects", sizeof(unsigned char) * textures_size * 4 + sizeof(RI_verticies) * uv_count * vs + sizeof(RI_triangles) * face_count * vs + sizeof(RI_verticies) * vertex_count * vs + sizeof(RI_verticies) * normal_count * vs + object_arary_size);

    debug(RI_DEBUG_HIGH, "clCreateBuffer object_arary_size: %d", object_arary_size);

    object_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, object_arary_size, objects, &error);
    erchk(error);
    
    if (object_memory_buffer == NULL){
        debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Objects cl_mem Buffer");
    }

    if (face_count > 0){
        triangles_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_triangles) * face_count * ts, triangles, &error);
        erchk(error);
    
        if (triangles_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Triangles cl_mem Buffer");
        }
    }

    if (vertex_count > 0){
        verticies_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * vertex_count * vs, verticies, &error);
        erchk(error);
    
        if (verticies_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Verticies cl_mem Buffer");
        }
    }

    if (normal_count > 0){
        normals_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * normal_count * vs, normals, &error);
        erchk(error);
    
        if (normals_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for Normals cl_mem Buffer");
        }
    }

    if (uv_count > 0){
        uvs_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RI_verticies) * uv_count * vs, uvs, &error);
        erchk(error);

        if (uvs_memory_buffer == NULL){
            debug(RI_DEBUG_LOW, "clCreateBuffer Failed for UVS cl_mem Buffer");
        }
    }

    debug(RI_DEBUG_MEDIUM, "Request for %d Objects Granted", object_count);
    
    debug(RI_DEBUG_HIGH, "Left RI_RequestObjects");

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
    
    debug_tick_func(1, "Called RI_Tick");
    
    if (running)
    {
        if (frame_buffer == NULL)
        {
            debug_tick_func(0, "Frame Buffer is not Allocated");
            return RI_ERROR;
        } 

        if (be_master_renderer){
            erchk(clSetKernelArg(compiled_kernel_master, 0, sizeof(cl_mem), &object_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 1, sizeof(cl_mem), &verticies_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 2, sizeof(cl_mem), &normals_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 3, sizeof(cl_mem), &uvs_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 4, sizeof(cl_mem), &triangles_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 5, sizeof(cl_mem), &output_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 6, sizeof(cl_mem), &textures_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 7, sizeof(cl_mem), &texture_info_memory_buffer));
            erchk(clSetKernelArg(compiled_kernel_master, 8, sizeof(int), (void*)&object_count));
            erchk(clSetKernelArg(compiled_kernel_master, 9, sizeof(int), (void*)&width));
            erchk(clSetKernelArg(compiled_kernel_master, 10, sizeof(int), (void*)&height));
            erchk(clSetKernelArg(compiled_kernel_master, 11, sizeof(int), (void*)&show_buffer)); 
            erchk(clSetKernelArg(compiled_kernel_master, 12, sizeof(int), (void*)&frame)); 

            if (object_count > 0) {
                erchk(clEnqueueWriteBuffer(queue, object_memory_buffer, CL_TRUE, 0, sizeof(float) * object_size * object_count, objects, 0, NULL, NULL));
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

            if (texture_count > 0){
                erchk(clEnqueueWriteBuffer(queue, textures_memory_buffer, CL_TRUE, 0, sizeof(unsigned char) * 4 * textures_size, textures, 0, NULL, NULL));
                erchk(clFinish(queue));

                erchk(clEnqueueWriteBuffer(queue, texture_info_memory_buffer, CL_TRUE, 0, sizeof(int) * tis * texture_count, texture_info, 0, NULL, NULL));
                erchk(clFinish(queue));

                debug_tick_func(1, "Wrote Textures Buffer and Texture Info Buffer");
            }
            
            erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * width * height, 0, NULL, NULL));
            erchk(clFinish(queue));
            debug_tick_func(1, "Cleared Frame Buffer");

            size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};

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
    debug(RI_DEBUG_HIGH, "Called RI_Stop");

    debug(RI_DEBUG_LOW, "Stopping...");

    running = 0;

    clReleaseMemObject(input_memory_buffer);
    clReleaseMemObject(output_memory_buffer);
    clReleaseMemObject(object_memory_buffer);
    clReleaseMemObject(verticies_memory_buffer);
    clReleaseMemObject(normals_memory_buffer);
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

    frame_buffer = malloc(sizeof(RI_uint) * width * height);

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

    if (width <= 0 || height <= 0)
    {
        debug(RI_DEBUG_LOW, "Invalid width or height");
        return RI_ERROR;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
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

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture)
    {
        debug(RI_DEBUG_LOW, "SDL_CreateTexture Failed");
        return RI_ERROR;
    }
    
    font = TTF_OpenFont(font_file, font_size);

    text_surface = TTF_RenderText_Solid(font, "FPS", font_color);
    if (text_surface == NULL){
        debug(RI_DEBUG_LOW, "TTF_RenderText_Solid Failed: %s", TTF_GetError());
        return RI_ERROR;
    }

    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (text_texture == NULL){
        debug(RI_DEBUG_LOW, "SDL_CreateTextureFromSurface Failed");
        return RI_ERROR;
    }

    debug(RI_DEBUG_LOW, "Initialized Rendering");

    return RI_SUCCESS;
}

RI_result OpenCL_init(){
    debug(RI_DEBUG_HIGH, "Called OpenCL_init");

    debug(RI_DEBUG_LOW, "Initialiing OpenCL...");

    clGetPlatformIDs(1, &platform, &number_of_platforms);

    if (number_of_platforms == 0)
    {
        debug(RI_DEBUG_LOW, "No OpenCL Platforms");
        return RI_ERROR;
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);

    if (number_of_devices == 0)
    {
        debug(RI_DEBUG_LOW, "No Valid GPU's Found");
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

    debug(RI_DEBUG_MEDIUM, "Local Size: %d", local_size);

    size_2d[0] = width;
    size_2d[1] = height;

    pattern = 0x22222222;

    debug(RI_DEBUG_LOW, "Initialized OpenCL");

    return RI_SUCCESS;
}

RI_result RI_Init(int RI_WindowWidth, int RI_WindowHeight, char *RI_WindowTitle){
    debug(RI_DEBUG_HIGH, "Called RI_Init");

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