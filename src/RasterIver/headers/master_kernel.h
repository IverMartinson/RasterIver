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
__kernel void raster_kernel(__global int* objects, __global float* verticies, __global float* normals, __global float* uvs, __global int* triangles, __global uint* frame_buffer, int object_count, int width, int height, int show_buffer){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    float highest_z = 800;\
    float lowest_z = 0;\
    \
    int has_normals = 1;\
    int has_uvs = 1;\
    float nx;\
    float ny;\
    float nz;\
    \
    float ux;\
    float uy;\
    float uz;\
    \
    float w0;\
float w1;\
float w2;\
    \
    float n_x0;\
float n_y0;\
float n_z0;\
            \
float n_x1;\
float n_y1;\
float n_z1;\
            \
float n_x2;\
float n_y2;\
float n_z2;\
            \
float u_x0;\
float u_y0;\
float u_z0;\
            \
float u_x1;\
float u_y1;\
float u_z1;\
            \
float u_x2;\
float u_y2;\
float u_z2;\
\
    for (int object = 0; object < object_count; object++){ \
        int base = object * 15;\
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
        int triangle_count = objects[base + 9];\
        int triangle_index = objects[base + 10];\
        int vertex_index = objects[base + 11];\
        int normal_index = objects[base + 12];\
        int uv_index = objects[base + 13];\
        int texture_index = objects[base + 14];\
        \
        for (int triangle = 0; triangle < triangle_count; triangle++){\
            int triangle_base = (triangle + triangle_index) * 9; \
\
            int i0 = (vertex_index + triangles[triangle_base + 0]) * 3;\
            int i1 = (vertex_index + triangles[triangle_base + 1]) * 3;\
            int i2 = (vertex_index + triangles[triangle_base + 2]) * 3;\
\
            int i3 = (normal_index + triangles[triangle_base + 3]) * 3;\
            int i4 = (normal_index + triangles[triangle_base + 4]) * 3;\
            int i5 = (normal_index + triangles[triangle_base + 5]) * 3;\
\
            int i6 = (uv_index + triangles[triangle_base + 6]) * 3;\
            int i7 = (uv_index + triangles[triangle_base + 7]) * 3;\
            int i8 = (uv_index + triangles[triangle_base + 8]) * 3;\
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
            \
            if (i3 < 0 || i4 < 0 || i5 < 0){\
                has_normals = 0;\
            }\
            if (i6 < 0 || i7 < 0 || i8 < 0){\
                has_uvs = 0;\
            }\
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
                \
                if (denominator <= 0) { \
                    continue; \
                } \
                w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; \
                w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y0)) / denominator; \
                w2 = 1.0 - w0 - w1; \
                \
                float z = w0 * z0 + w1 * z1 + w2 * z2; \
                \
                if (z > z_pixel){ \
                    z_pixel = z; \
                    \
            n_x0 = normals[i3 + 0];\
            n_y0 = normals[i3 + 1];\
            n_z0 = normals[i3 + 2];\
            \
            n_x1 = normals[i4 + 0];\
            n_y1 = normals[i4 + 1];\
            n_z1 = normals[i4 + 2];\
            \
            n_x2 = normals[i5 + 0];\
            n_y2 = normals[i5 + 1];\
            n_z2 = normals[i5 + 2];\
            \
            u_x0 = uvs[i6 + 0];\
            u_y0 = uvs[i6 + 1];\
            u_z0 = uvs[i6 + 2];\
            \
            u_x1 = uvs[i7 + 0];\
            u_y1 = uvs[i7 + 1];\
            u_z1 = uvs[i7 + 2];\
            \
            u_x2 = uvs[i8 + 0];\
            u_y2 = uvs[i8 + 1];\
            u_z2 = uvs[i8 + 2];\
                    \
                    frame_pixel = 0xFFFFFFFF;\
                } \
            }\
        }\
    }\
    \
    if (id_y * width + id_x >= width * height){\
        return;\
    }\
    frame_buffer[(height - id_y) * width + id_x] = frame_pixel; \
    \
    switch (show_buffer){\
        case 1:{\
            float z = clamp(z_pixel, 0.0f, highest_z);\
            \
            float norm_z = z / highest_z;\
            \
            uchar intensity = (uchar)(norm_z * 255.0f);\
            \
            frame_buffer[(height - id_y) * width + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
            \
            break;}\
        case 2:{\
        nx = w0 * n_x0 + w1 * n_x1 + w2 * n_x2;\
                    ny = w0 * n_y0 + w1 * n_y1 + w2 * n_y2;\
                    nz = w0 * n_z0 + w1 * n_z1 + w2 * n_z2;\
            nx = clamp((nx * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);\
            ny = clamp((ny * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);\
            nz = clamp((nz * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);\
            \
            uchar r = (uchar)nx;\
            uchar g = (uchar)ny;\
            uchar b = (uchar)nz;\
            \
            if (!has_normals){\
                r = 20;\
                g = 20;\
                b = 20;\
            }\
            \
            frame_buffer[(height - id_y) * width + id_x] = 0xFF000000 | (r << 16) | (g << 8) | b;\
            \
            break;}\
        case 3:{\
            ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;\
            uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;\
            \
            ux = clamp(ux * 255.0f, 0.0f, 255.0f);\
            uy = clamp(uy * 255.0f, 0.0f, 255.0f);\
            \
            uchar r = (uchar)ux;\
            uchar g = (uchar)uy;\
            uchar b = 0;\
            \
            if (!has_uvs){\
                r = 20;\
                g = 20;\
                b = 20;\
            }\
            \
            frame_buffer[(height - id_y) * width + id_x] = 0xFF000000 | (r << 16) | (g << 8) | b;\
            \
            break;}\
        default:{\
            break;}\
    }\
}\n";
