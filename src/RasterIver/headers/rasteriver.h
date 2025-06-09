#ifndef RASTERIVER_H
#define RASTERIVER_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdint.h>
#include "../kernels/master_kernel.h"
#include "../kernels/transformer.h"
#include "../kernels/non_master_kernel.h"
#include <SDL2/SDL.h>

typedef int RI_result;
typedef int RI_flag;
typedef uint32_t RI_uint;
typedef float* RI_polygons;
typedef float* RI_verticies;
typedef int* RI_triangles;
typedef float* RI_objects;
typedef unsigned char* RI_textures;

// the size of each object instance in the objects array
// xyz + rot(xyz) + scale(xyz) = 9
// 9 + polygon count + polygon index + vertex index + normal index + UV index + texture index + rotation w = 16
#define object_size 16

// vertex size
// the size of each instance of verticies
#define vs 3

// triangle size
// 3 xyz 3 normals 3 uvs 
#define ts 9

// texture info size
// width + height + value offset = 3
#define tis 3

// PI
#define RI_PI 3.14159265359

typedef struct {
    float x, y, z, r_x, r_y, r_z, r_w, s_x, s_y, s_z;
    char file_path[40];
    char texture[40];
} RI_newObject;

typedef struct {
    int verticies, normals, uvs, faces, length_of_texture;
} load_object_return;

// RI_result
typedef enum {
    RI_ERROR        = -1,
    RI_SUCCESS      =  0,
    RI_NOT_RUNNING  = -2,
    RI_RUNNING      =  1,
    RI_INVALID_FLAG = -3,
} RI_result_enum;

// RI_flag
typedef enum {
    RI_FLAG_DEBUG               = 0,
    RI_FLAG_DEBUG_LEVEL         = 1,
    RI_FLAG_SHOW_BUFFER         = 2,
    RI_FLAG_SHOW_FPS            = 3,
    RI_FLAG_DEBUG_FPS           = 4,
    RI_FLAG_CLEAN_POLYGONS      = 5,
    RI_FLAG_POPULATE_POLYGONS   = 6,
    RI_FLAG_BE_MASTER_RENDERER  = 7,
    RI_FLAG_DEBUG_FRAME         = 8,
    RI_FLAG_SHOW_FRAME          = 9,
    RI_FLAG_SHOW_INFO           = 10,
    RI_FLAG_DEBUG_TICK          = 11,
    RI_FLAG_USE_CPU             = 12,
    RI_FLAG_HANDLE_SDL_EVENTS   = 13,
} RI_flag_enum;

// RI_BUFFER
typedef enum {
    RI_BUFFER_COMPLETE  = 0,
    RI_BUFFER_Z         = 1,
    RI_BUFFER_NORMAL    = 2,
    RI_BUFFER_UV        = 3,
} RI_buffer_enum;

// RI_DEBUG
typedef enum {
    RI_DEBUG_LOW      = 0,
    RI_DEBUG_MEDIUM   = 1,
    RI_DEBUG_HIGH     = 2,
} RI_debug_enum;

// RI_VALUE
typedef enum {
    RI_TRUE      = 1,
    RI_FALSE   = 0,
} RI_value_enum;

// Initializes OpenCL, SDL2, and anything else Rasteriver needs to function
RI_result   RI_Init();

// Safely stops Rasteriver
RI_result   RI_Stop();

// Checks if Rasteriver is still running and returns if it is
RI_result   RI_IsRunning();

// Asks Rasteriver to allocate space for a number of polygons
// Returns a pointer to the polygons array
RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest);

// Asks Rasteriver to allocate and load objects
// Returns a pointer to the objects array
RI_objects  RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest);

// Ticks Rasteriver
RI_result   RI_Tick();

// Sets the background color
RI_result   RI_SetBackground(RI_uint RI_BackgroundColor);

// Depricated
// Shows the Z buffer
RI_result   RI_ShowZBuffer(int RI_ShowZBufferFlag);

// Sets a flag
RI_result   RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value);

// Sets the FPS limit
RI_result   RI_SetFpsCap(int RI_FpsCap);

// Depricated
// Returns a list of Rasteriver flags
RI_result   RI_ListFlags();

// Sets the prefix when debugging
// Default is [RASTERIVER] 
RI_result RI_SetDebugPrefix(char *RI_Prefix);

#endif // RASTERIVER_H