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
__kernel void raster_kernel(__global int* objects, __global float* verticies, __global float* normals, __global float* uvs, __global int* triangles, __global uint* frame_buffer, int object_count, int width, int height, int show_z_buffer, float highest_z){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    float biggest_z = 0;\
    \
    for (int object = 0; object < object_count; object++){ \
        int base = object * 13;\
        \
        int object_x =   objects[base + 0]; \
        int object_y =   objects[base + 1]; \
        int object_z =   objects[base + 2]; \
        int object_r_x = objects[base + 3]; \
        int object_r_y = objects[base + 4]; \
        int object_r_z = objects[base + 5]; \
        int object_s_x = objects[base + 6]; \
        int object_s_y = objects[base + 7]; \
        int object_s_z = objects[base + 8]; \
        \
        int polygon_count = objects[base + 9];\
        int polygon_offset = objects[base + 10];\
        int vertex_offset = objects[base + 11];\
        int texture_index = objects[base + 12];\
        \
        for (int polygon = polygon_offset; polygon < polygon_count + polygon_offset; polygon++){\
            int polygon_base = polygon * 9; \
\
            int i0 = triangles[polygon_base + 0] * 3 + vertex_offset;\
            int i1 = triangles[polygon_base + 1] * 3 + vertex_offset;\
            int i2 = triangles[polygon_base + 2] * 3 + vertex_offset;\
            \
            float x0 = verticies[i0 + 0] * object_s_x + object_x;\
            float y0 = verticies[i0 + 1] * object_s_y + object_y;\
            float z0 = verticies[i0 + 2] * object_s_z + object_z;\
            \
            float x1 = verticies[i1 + 0] * object_s_x + object_x;\
            float y1 = verticies[i1 + 1] * object_s_y + object_y;\
            float z1 = verticies[i1 + 2] * object_s_z + object_z;\
            \
            float x2 = verticies[i2 + 0] * object_s_x + object_x;\
            float y2 = verticies[i2 + 1] * object_s_y + object_y;\
            float z2 = verticies[i2 + 2] * object_s_z + object_z;\
            \
            if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){\
                continue;\
            }\
            \
            float smallest_x = x0; \
            float largest_x = x0; \
            float smallest_y = y0; \
            float largest_y = y0; \
            \
            if (x0 > largest_x) largest_x = x0;\
            if (x1 > largest_x) largest_x = x1;\
            if (x2 > largest_x) largest_x = x2;\
            \
            if (x0 < smallest_x) smallest_x = x0;\
            if (x1 < smallest_x) smallest_x = x1;\
            if (x2 < smallest_x) smallest_x = x2;\
            \
            if (y0 > largest_y) largest_y = y0;\
            if (y1 > largest_y) largest_y = y1;\
            if (y2 > largest_y) largest_y = y2;\
            \
            if (y0 < smallest_y) smallest_y = y0;\
            if (y1 < smallest_y) smallest_y = y1;\
            if (y2 < smallest_y) smallest_y = y2;\
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
        }\
    }\
    \
    if (id_y * width + id_x >= width * height){\
        return;\
    }\
    frame_buffer[id_y * width + id_x] = frame_pixel; \
    \
    if (!show_z_buffer){\
        return;\
    }\
    \
    float z = clamp(z_pixel, 0.0f, highest_z);\
    \
    float norm_z = z / highest_z;\
    \
    uchar intensity = (uchar)(norm_z * 255.0f);\
    \
    frame_buffer[id_y * width + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
}\n";
