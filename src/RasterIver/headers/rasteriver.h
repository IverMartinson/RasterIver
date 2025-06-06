#ifndef RASTERIVER_H
#define RASTERIVER_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdint.h>
#include "../kernels/master_kernel.h"
#include "../kernels/non_master_kernel.h"

typedef int RI_result;
typedef int RI_flag;
typedef uint32_t RI_uint;
typedef float* RI_polygons;
typedef float* RI_verticies;
typedef int* RI_triangles;
typedef int* RI_objects;
typedef unsigned char* RI_textures;

// the size of each object instance in the objects array
// xyz + rot(xyz) + scale(xyz) = 9
// 9 + polygon count + polygon index + vertex index + normal index + UV index + texture index = 15
#define object_size 15

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
    float x, y, z, r_x, r_y, r_z, s_x, s_y, s_z;
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

RI_result   RI_Init();
RI_result   RI_Stop();
RI_result   RI_IsRunning();
RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest);
RI_objects  RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest);
RI_result   RI_Tick();
RI_result   RI_SetBackground(RI_uint RI_BackgroundColor);
RI_result   RI_ShowZBuffer(int RI_ShowZBufferFlag);
RI_result   RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value);
RI_result   RI_SetFpsCap(int RI_FpsCap);
RI_result   RI_ListFlags();

#endif // RASTERIVER_H