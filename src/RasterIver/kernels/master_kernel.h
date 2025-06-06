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
inline int clamppp(int x, int lower, int upper) {\
    return x < lower ? lower : (x > upper ? upper : x);\
}\
\
void rotate_quaternion(float *x, float *y, float *z, float r_x, float r_y, float r_z){\
\
};\
\
void rotate_euler(float *x, float *y, float *z, float r_x, float r_y, float r_z){\
    float cx = cos(r_x), sx = sin(r_x);\
    float cy = cos(r_y), sy = sin(r_y);\
    float cz = cos(r_z), sz = sin(r_z);\
\
    float matrix[3][3] = {\
        {\
            cy * cz,\
            -cy * sz,\
            sy\
        },\
        {\
            sx * sy * cz + cx * sz,\
            -sx * sy * sz + cx * cz,\
            -sx * cy\
        },\
        {\
            -cx * sy * cz + sx * sz,\
            cx * sy * sz + sx * cz,\
            cx * cy\
        }\
    };\
    \
    float temp_x = matrix[0][0] * *x + matrix[0][1] * *y + matrix[0][2] * *z;\
    float temp_y = matrix[1][0] * *x + matrix[1][1] * *y + matrix[1][2] * *z;\
    float temp_z = matrix[2][0] * *x + matrix[2][1] * *y + matrix[2][2] * *z;\
\
    *x = temp_x;\
    *y = temp_y;\
    *z = temp_z;\
};\
\
__kernel void raster_kernel(__global float* objects, __global float* verticies, __global float* normals, __global float* uvs, __global int* triangles, __global uint* frame_buffer, __global uchar* textures, __global int* texture_info, int object_count, int width, int height, int show_buffer, int frame){ \
    int id_x = get_global_id(0) - width / 2; \
    int id_y = get_global_id(1) - height / 2; \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    float highest_z = 800;\
    float lowest_z = 0;\
    \
    int has_normals = 1;\
    int has_uvs = 1;\
    \
    float w0;\
    float w1;\
    float w2;\
    \
\
    for (int object = 0; object < object_count; object++){ \
        int base = object * 16;\
        \
        float object_x =   objects[base + 0]; \
        float object_y =   objects[base + 1]; \
        float object_z =   objects[base + 2]; \
        float object_r_x = objects[base + 3]; \
        float object_r_y = objects[base + 4]; \
        float object_r_z = objects[base + 5]; \
        float object_r_w = objects[base + 15]; \
        float object_s_x = objects[base + 6]; \
        float object_s_y = objects[base + 7]; \
        float object_s_z = objects[base + 8]; \
        \
        int triangle_count = (int)objects[base + 9];\
        int triangle_index = (int)objects[base + 10];\
        int vertex_index =   (int)objects[base + 11];\
        int normal_index =   (int)objects[base + 12];\
        int uv_index =       (int)objects[base + 13];\
        int texture_index =  (int)objects[base + 14];\
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
            float z0 = verticies[i0 + 2];\
            float x0 = verticies[i0 + 0];\
            float y0 = verticies[i0 + 1];\
            \
            float z1 = verticies[i1 + 2];\
            float x1 = verticies[i1 + 0];\
            float y1 = verticies[i1 + 1];\
            \
            float z2 = verticies[i2 + 2];\
            float x2 = verticies[i2 + 0];\
            float y2 = verticies[i2 + 1];\
            \
            if (object_r_w <= -9999999){\
                rotate_euler(&x0, &y0, &z0, object_r_x, object_r_y, object_r_z);\
                rotate_euler(&x1, &y1, &z1, object_r_x, object_r_y, object_r_z);\
                rotate_euler(&x2, &y2, &z2, object_r_x, object_r_y, object_r_z);\
            }\
            else{\
                rotate_euler(&x0, &y0, &z0, object_r_x, object_r_y, object_r_z);\
                rotate_euler(&x1, &y1, &z1, object_r_x, object_r_y, object_r_z);\
                rotate_euler(&x2, &y2, &z2, object_r_x, object_r_y, object_r_z);\
            }\
            \
            z0 = z0 * object_s_z + object_z;\
            x0 = x0 * object_s_x + object_x;\
            y0 = y0 * object_s_y + object_y;\
            z1 = z1 * object_s_z + object_z;\
            x1 = x1 * object_s_x + object_x;\
            y1 = y1 * object_s_y + object_y;\
            z2 = z2 * object_s_z + object_z;\
            x2 = x2 * object_s_x + object_x;\
            y2 = y2 * object_s_y + object_y;\
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
                    float n_x0 = normals[i3 + 0];\
                    float n_y0 = normals[i3 + 1];\
                    float n_z0 = normals[i3 + 2];\
                    \
                    float n_x1 = normals[i4 + 0];\
                    float n_y1 = normals[i4 + 1];\
                    float n_z1 = normals[i4 + 2];\
                    \
                    float n_x2 = normals[i5 + 0];\
                    float n_y2 = normals[i5 + 1];\
                    float n_z2 = normals[i5 + 2];\
                    \
                    float u_x0 = uvs[i6 + 0];\
                    float u_y0 = uvs[i6 + 1];\
                    float u_z0 = uvs[i6 + 2];\
                    \
                    float u_x1 = uvs[i7 + 0];\
                    float u_y1 = uvs[i7 + 1];\
                    float u_z1 = uvs[i7 + 2];\
                    \
                    float u_x2 = uvs[i8 + 0];\
                    float u_y2 = uvs[i8 + 1];\
                    float u_z2 = uvs[i8 + 2];\
                    \
                    if (object_r_w <= -9999999){\
                        rotate_euler(&n_x0, &n_y0, &n_z0, object_r_x, object_r_y, object_r_z);\
                        rotate_euler(&n_x1, &n_y1, &n_z1, object_r_x, object_r_y, object_r_z);\
                        rotate_euler(&n_x2, &n_y2, &n_z2, object_r_x, object_r_y, object_r_z);\
                    }\
                    else{\
                        rotate_euler(&n_x0, &n_y0, &n_z0, object_r_x, object_r_y, object_r_z);\
                        rotate_euler(&n_x1, &n_y1, &n_z1, object_r_x, object_r_y, object_r_z);\
                        rotate_euler(&n_x2, &n_y2, &n_z2, object_r_x, object_r_y, object_r_z);\
                    }\
                    \
                    switch (show_buffer){\
                        case 0:{\
                            double ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;\
                            double uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;\
                            \
                            int texture_width = texture_info[texture_index * 3];\
                            int texture_height = texture_info[texture_index * 3 + 1];\
                            int texture_value_offset = texture_info[texture_index * 3 + 2];\
                            \
                            int ix = max((int)(ux * texture_width), 0);\
                            int iy = max((int)(uy * texture_height), 0);\
                            \
                            int uv_pixel = (iy * texture_width + ix) * 4 + texture_value_offset;\
                            \
                            uchar r = textures[uv_pixel + 0];\
                            uchar g = textures[uv_pixel + 1];\
                            uchar b = textures[uv_pixel + 2];\
                            uchar a = textures[uv_pixel + 3];\
                            \
                            frame_pixel = (a << 24) | (r << 16) | (g << 8) | b;\
                            \
                            break;}\
                        case 1:{\
                            float z = clamp(z_pixel, 0.0f, highest_z);\
                            \
                            float norm_z = z / highest_z;\
                            \
                            uchar intensity = (uchar)(norm_z * 255.0f);\
                            \
                            frame_pixel = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
                            \
                            break;}\
                        case 2:{\
                            float nx = w0 * n_x0 + w1 * n_x1 + w2 * n_x2;\
                            float ny = w0 * n_y0 + w1 * n_y1 + w2 * n_y2;\
                            float nz = w0 * n_z0 + w1 * n_z1 + w2 * n_z2;\
                            \
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
                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;\
                            \
                            break;}\
                        case 3:{\
                            float ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;\
                            float uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;\
                            \
                            uchar r = (uchar)clamp(ux * 255.0f, 0.0f, 255.0f);\
                            uchar g = (uchar)clamp(uy * 255.0f, 0.0f, 255.0f);\
                            uchar b = 0;\
                            \
                            if (!has_uvs){\
                                r = 20;\
                                g = 20;\
                                b = 20;\
                            }\
                            \
                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;\
                            \
                            break;}\
                        default:{\
                            frame_pixel = 0xFF00FFFF;\
                            \
                            break;}\
                    }\
                } \
            }\
        }\
    }\
    \
    int pixel_coord = (height * 0.5 - id_y) * width + id_x + width * 0.5;\
    \
    if (pixel_coord >= width * height || pixel_coord < 0){\
        return;\
    }\
    \
    frame_buffer[pixel_coord] = frame_pixel; \
    \
}\n";
