const char *kernel_source_transformer = " \
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
__kernel void transformer_kernel(__global float* objects, __global float* verticies, __global float* normals, __global float* triangles, __global float* transformed_verticies, __global float* transformed_normals, float fov){ \
    int id_x = get_global_id(0);\
    \
    float vertical_fov_factor = height / tan(0.5 * fov);\
    float horizontal_fov_factor = width / tan(0.5 * fov);\
    \
    int has_normals = 1;\
    int has_uvs = 1;\
    \
    int base = id_x * 16;\
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
        float z1 = verticies[i1 + 2];\
        float x1 = verticies[i1 + 0];\
        float y1 = verticies[i1 + 1];\
        float z2 = verticies[i2 + 2];\
        float x2 = verticies[i2 + 0];\
        float y2 = verticies[i2 + 1];\
        float n_x0 = normals[i3 + 0];\
        float n_y0 = normals[i3 + 1];\
        float n_z0 = normals[i3 + 2];\
        float n_x1 = normals[i4 + 0];\
        float n_y1 = normals[i4 + 1];\
        float n_z1 = normals[i4 + 2];\
        float n_x2 = normals[i5 + 0];\
        float n_y2 = normals[i5 + 1];\
        float n_z2 = normals[i5 + 2];\
        \
        if (object_r_w <= -9999999){\
            rotate_euler(&x0, &y0, &z0, object_r_x, object_r_y, object_r_z);\
            rotate_euler(&x1, &y1, &z1, object_r_x, object_r_y, object_r_z);\
            rotate_euler(&x2, &y2, &z2, object_r_x, object_r_y, object_r_z);\
            \    
            rotate_euler(&n_x0, &n_y0, &n_z0, object_r_x, object_r_y, object_r_z);\
            rotate_euler(&n_x1, &n_y1, &n_z1, object_r_x, object_r_y, object_r_z);\
            rotate_euler(&n_x2, &n_y2, &n_z2, object_r_x, object_r_y, object_r_z);\
        }\
        else{\
        }\
        \
        transformed_verticies[i0 + 2] = (z0 * object_s_z + object_z);\
        transformed_verticies[i0 + 0] = (x0 * object_s_x + object_x) / z0 * horizontal_fov_factor;\
        transformed_verticies[i0 + 1] = (y0 * object_s_y + object_y) / z0 * vertical_fov_factor;\
        transformed_verticies[i1 + 2] = (z1 * object_s_z + object_z);\
        transformed_verticies[i1 + 0] = (x1 * object_s_x + object_x) / z1 * horizontal_fov_factor;\
        transformed_verticies[i1 + 1] = (y1 * object_s_y + object_y) / z1 * vertical_fov_factor;\
        transformed_verticies[i2 + 2] = (z2 * object_s_z + object_z);\
        transformed_verticies[i2 + 0] = (y2 * object_s_y + object_y) / z2 * horizontal_fov_factor;\
        transformed_verticies[i2 + 1] = (x2 * object_s_x + object_x) / z2 * vertical_fov_factor;\
        \
        transformed_normals[i3 + 0] = n_x0;\
        transformed_normals[i3 + 1] = n_y0;\
        transformed_normals[i3 + 2] = n_z0;\
        transformed_normals[i4 + 0] = n_x1;\
        transformed_normals[i4 + 1] = n_y1;\
        transformed_normals[i4 + 2] = n_z1;\
        transformed_normals[i5 + 0] = n_x2;\
        transformed_normals[i5 + 1] = n_y2;\
        transformed_normals[i5 + 2] = n_z2;\
        \
        if (i3 < 0 || i4 < 0 || i5 < 0){\
            has_normals = 0;\
        }\
        \
        if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){\
            continue;\
        }\
    }\
}\n";
