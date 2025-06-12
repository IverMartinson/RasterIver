#ifndef RASTERIVER_H
#define RASTERIVER_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdint.h>
#include <SDL2/SDL.h>
#include "object.h"

typedef int RI_result;
typedef int RI_flag;
typedef int RI_value;
typedef uint32_t RI_uint;
typedef float* RI_polygons;
typedef float* RI_verticies;
typedef int* RI_triangles;
typedef Object* RI_objects;
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
#define RI_2PI 6.28318530718

typedef struct {
    float x, y, z, r_x, r_y, r_z, r_w, s_x, s_y, s_z;
    uint64_t material_flags;
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
    RI_INVALID_VALUE = -4,
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

typedef enum {
    RI_VALUE_WIREFRAME_SCALE = 0,
} RI_value_enum;

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

typedef enum {
    RI_MATERIAL_UNLIT = ((uint64_t)1 << 0), // should calculate lighting
    RI_MATERIAL_DONT_CAST_SHADOW = ((uint64_t)1 << 1), // should cast shadow on other objects
    RI_MATERIAL_HAS_TEXTURE = ((uint64_t)1 << 2), // has a texture
    RI_MATERIAL_HAS_NORMAL_MAP = ((uint64_t)1 << 3), // has a normal map
    RI_MATERIAL_HAS_BUMP_MAP = ((uint64_t)1 << 4), // has a bump map
    RI_MATERIAL_TRANSPARENT = ((uint64_t)1 << 5), // has transparency
    RI_MATERIAL_WIREFRAME = ((uint64_t)1 << 6), // render as wireframe
    RI_MATERIAL_DONT_RECEIVE_SHADOW = ((uint64_t)1 << 7), // should shadows render on it
    RI_MATERIAL_DONT_DEPTH_TEST = ((uint64_t)1 << 8), // should check Z buffer (if 1, render on top of everything)
    RI_MATERIAL_DONT_DEPTH_WRITE = ((uint64_t)1 << 9), // should write to the Z buffer (if 1, render behind everything)
    RI_MATERIAL_DOUBLE_SIDED = ((uint64_t)1 << 10), // ignore backface culling
} RI_material_properties_enum;

typedef enum {
    RI_PMP_TEXTURED = RI_MATERIAL_HAS_TEXTURE,
} RI_preset_material_properties;

// RI_BOOL
typedef enum {
    RI_TRUE      = 1,
    RI_FALSE   = 0,
} RI_bool_enum;

// Initializes OpenCL, SDL2, and anything else Rasteriver needs to function
RI_result   RI_Init();

// Safely stops Rasteriver
RI_result   RI_Stop();

// Checks if Rasteriver is still running and returns if it is
RI_result   RI_IsRunning();

// Asks Rasteriver to allocate space for a number of polygons
// Returns a pointer to the new object
int* RI_RequestMesh(int RI_PolygonsToRequest);

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

// Sets a value
RI_result   RI_SetValue(RI_value RI_ValueToSet, float RI_Value);

// Sets the FPS limit
RI_result   RI_SetFpsCap(int RI_FpsCap);

// Depricated
// Returns a list of Rasteriver flags
RI_result   RI_ListFlags();

// Sets the prefix when debugging
// Takes a 49 char string (50 including \0)
// Default is [RasterIver] 
RI_result RI_SetDebugPrefix(char RI_Prefix[50]);

#endif // RASTERIVER_H