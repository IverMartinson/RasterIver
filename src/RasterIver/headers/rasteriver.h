#ifndef RASTERIVER_H
#define RASTERIVER_H

#include <stdint.h>

typedef int RI_result;
typedef int RI_flag;
typedef uint32_t RI_uint;
typedef float* RI_polygons;
typedef float* RI_verticies;
typedef int* RI_triangles;
typedef int* RI_objects;
typedef char* RI_textures;

// the size of each object instance in the objects array
// xyz + rot(xyz) + scale(xyz) = 9
// 9 + polygon count + polygon index + texture index = 12
#define object_size 12

// vertex size
// the size of each instance of verticies
#define vs 3

// triangle size
// 3 xyz 3 normals 3 uvs 
#define ts 9

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
    RI_FLAG_DEBUG_VERBOSE       = 1,
    RI_FLAG_SHOW_Z_BUFFER       = 2,
    RI_FLAG_SHOW_FPS            = 3,
    RI_FLAG_DEBUG_FPS           = 4,
    RI_FLAG_CLEAN_POLYGONS      = 5,
    RI_FLAG_POPULATE_POLYGONS   = 6,
    RI_FLAG_BE_MASTER_RENDERER  = 7,
} RI_flag_enum;

RI_result   RI_Init();
RI_result   RI_Stop();
RI_result   RI_IsRunning();
RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest);
RI_result   RI_RequestObjects(RI_newObject *RI_ObjectBuffer, int RI_ObjectsToRequest);
RI_result   RI_Tick();
RI_result   RI_SetBackground(RI_uint RI_BackgroundColor);
RI_result   RI_ShowZBuffer(int RI_ShowZBufferFlag);
RI_result   RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value);
RI_result   RI_SetFpsCap(int RI_FpsCap);
RI_result   RI_ListFlags();

const char *kernel_source_non_master = " \
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
\
void norm(float dest[2], float a[2]){ \
    float magnitude = sqrt(a[0] * a[0] + a[1] * a[1]); \
    \
    dest[0] = a[0] / magnitude; \
    dest[1] = a[1] / magnitude; \
    } \
    \
    void sub(float dest[2], float a[2], float b[2]){ \
    dest[0] = a[0] - b[0]; \
    dest[1] = a[1] - b[1]; \
    } \
    \
    void add(float dest[2], float a[2], float b[2]){ \
    dest[0] = a[0] + b[0]; \
    dest[1] = a[1] + b[1]; \
} \
\
__kernel void raster_kernel(__global float* polygons, __global uint* frame_buffer, int polygon_count, int width, int height, int show_z_buffer, float highest_z){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    float biggest_z = 0;\
    \
    for (int polygon = 0; polygon < polygon_count; polygon++){ \
        int base = polygon * 9; \
        float x0 = polygons[base]; \
        float y0 = polygons[base + 1]; \
        float z0 = polygons[base + 2]; \
        float x1 = polygons[base + 3]; \
        float y1 = polygons[base + 4]; \
        float z1 = polygons[base + 5]; \
        float x2 = polygons[base + 6]; \
        float y2 = polygons[base + 7]; \
        float z2 = polygons[base + 8]; \
        \
        if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){\
            return;\
        }\
        \
        float smallest_x = x0; \
        float largest_x = x0; \
        float smallest_y = y0; \
        float largest_y = y0; \
        \
        for (int point = 0; point < 3; point++){ \
            float x = polygons[base + point * 3]; \
            float y = polygons[base + point * 3 + 1]; \
            \
            if (x > largest_x){ \
                largest_x = x; \
            } \
            \
            if (x < smallest_x){ \
                smallest_x = x; \
            } \
            \
            if (y > largest_y){ \
                largest_y = y; \
            } \
            \
            if (y < smallest_y){\
                smallest_y = y;\
            } \
        } \
        \
        smallest_x = fmin(smallest_x, 0); \
        largest_x = fmax(largest_x, width); \
        smallest_y = fmin(smallest_y, 0); \
        largest_y = fmax(largest_y, height); \
        \
        if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ \
            int intersections = 0; \
            \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x0, y0, x1, y1); \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x1, y1, x2, y2); \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x2, y2, x0, y0); \
            \
            if (intersections % 2 == 0){ \
                continue; \
            } \
            \
            float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); \
            float w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; \
            float w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y2)) / denominator; \
            float w2 = 1.0 - w0 - w1; \
            \
            if (denominator < 0) { \
                w0 = -w0; \
                w1 = -w1; \
                w2 = -w2; \
                denominator = -denominator; \
            } \
            \
            float z = w0 * z0 + w1 * z1 + w2 * z2; \
            \
            if (z < 0){ \
                z *= -1; \
            } \
            \
            if (z > z_pixel){ \
                z_pixel = z; \
            } \
             \
            else { \
                continue; \
            } \
            \
            frame_pixel = 0xFFFFFFFF / polygon_count * (polygon + 1); \
        } \
    } \
    \
    if (id_y * width + id_x > width * height){\
    return;\
    }\
    frame_buffer[id_y * width + id_x] = frame_pixel; \
    \
    if (!show_z_buffer){return;}\
    \
    float z = clamp(z_pixel, 0.0f, highest_z);\
    \
    float norm_z = z / highest_z;\
    \
    uchar intensity = (uchar)(norm_z * 255.0f);\
    \
    frame_buffer[id_y * width + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
}\n";

const char *kernel_source_master = " \
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
\
void norm(float dest[2], float a[2]){ \
    float magnitude = sqrt(a[0] * a[0] + a[1] * a[1]); \
    \
    dest[0] = a[0] / magnitude; \
    dest[1] = a[1] / magnitude; \
    } \
    \
    void sub(float dest[2], float a[2], float b[2]){ \
    dest[0] = a[0] - b[0]; \
    dest[1] = a[1] - b[1]; \
    } \
    \
    void add(float dest[2], float a[2], float b[2]){ \
    dest[0] = a[0] + b[0]; \
    dest[1] = a[1] + b[1]; \
} \
\
__kernel void raster_kernel(__global float* polygons, __global uint* frame_buffer, int polygon_count, int width, int height, int show_z_buffer, float highest_z){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    float biggest_z = 0;\
    \
    for (int polygon = 0; polygon < polygon_count; polygon++){ \
        int base = polygon * 9; \
        float x0 = polygons[base]; \
        float y0 = polygons[base + 1]; \
        float z0 = polygons[base + 2]; \
        float x1 = polygons[base + 3]; \
        float y1 = polygons[base + 4]; \
        float z1 = polygons[base + 5]; \
        float x2 = polygons[base + 6]; \
        float y2 = polygons[base + 7]; \
        float z2 = polygons[base + 8]; \
        \
        if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){\
            return;\
        }\
        \
        float smallest_x = x0; \
        float largest_x = x0; \
        float smallest_y = y0; \
        float largest_y = y0; \
        \
        for (int point = 0; point < 3; point++){ \
            float x = polygons[base + point * 3]; \
            float y = polygons[base + point * 3 + 1]; \
            \
            if (x > largest_x){ \
                largest_x = x; \
            } \
            \
            if (x < smallest_x){ \
                smallest_x = x; \
            } \
            \
            if (y > largest_y){ \
                largest_y = y; \
            } \
            \
            if (y < smallest_y){\
                smallest_y = y;\
            } \
        } \
        \
        smallest_x = fmin(smallest_x, 0); \
        largest_x = fmax(largest_x, width); \
        smallest_y = fmin(smallest_y, 0); \
        largest_y = fmax(largest_y, height); \
        \
        if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ \
            int intersections = 0; \
            \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x0, y0, x1, y1); \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x1, y1, x2, y2); \
            intersections += is_intersecting(id_x, id_y, 10000, 100000, x2, y2, x0, y0); \
            \
            if (intersections % 2 == 0){ \
                continue; \
            } \
            \
            float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); \
            float w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; \
            float w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y2)) / denominator; \
            float w2 = 1.0 - w0 - w1; \
            \
            if (denominator < 0) { \
                w0 = -w0; \
                w1 = -w1; \
                w2 = -w2; \
                denominator = -denominator; \
            } \
            \
            float z = w0 * z0 + w1 * z1 + w2 * z2; \
            \
            if (z < 0){ \
                z *= -1; \
            } \
            \
            if (z > z_pixel){ \
                z_pixel = z; \
            } \
             \
            else { \
                continue; \
            } \
            \
            frame_pixel = 0xFFFFFFFF / polygon_count * (polygon + 1); \
        } \
    } \
    \
    if (id_y * width + id_x > width * height){\
    return;\
    }\
    frame_buffer[id_y * width + id_x] = frame_pixel; \
    \
    if (!show_z_buffer){return;}\
    \
    float z = clamp(z_pixel, 0.0f, highest_z);\
    \
    float norm_z = z / highest_z;\
    \
    uchar intensity = (uchar)(norm_z * 255.0f);\
    \
    frame_buffer[id_y * width + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
}\n";

#endif // RASTERIVER_H