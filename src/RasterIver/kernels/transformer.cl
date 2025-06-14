static inline float minnn(float a, float b) {
    return (a < b) ? a : b;
}

static inline float maxxx(float a, float b) {
    return (a > b) ? a : b;
}

typedef struct {
    float x;
    float y;
    float z;
    float _pad0;
} Vec3;

typedef struct {
    float w;
    float x;
    float y;
    float z;
} Vec4;

typedef struct {
    uchar a;
    uchar r;
    uchar g;
    uchar b;
} ColorARGB;

typedef struct {
    ColorARGB albedo;
    int textureOffset;
    int normalMapOffset;
    int bumpMapOffset;
    ulong properties;
} Material;

typedef struct {
    Vec3 position;
    Vec3 scale;
    Vec4 rotation;
} Transform;

typedef struct {
    int transformedVertexOffset;
    int transformedNormalOffset;
    int triangleCount;
    int vertexCount;
    int normalCount;
    int uvCount;
    int triangleOffset;
    int vertexOffset;
    int normalOffset;
    int uvOffset;
} ModelInfo;

typedef struct {
    Transform transform;
    ModelInfo modelInfo;
    int id;
    int _pad1;
    Material material;
    int _pad2;
    int _pad3;
} Object;

inline int clamppp(int x, int lower, int upper) {
    return x < lower ? lower : (x > upper ? upper : x);
}

void rotate_quaternion(float *x, float *y, float *z, float r_x, float r_y, float r_z){

};

void rotate_euler(float *x, float *y, float *z, float r_x, float r_y, float r_z){
    float cx = cos(r_x), sx = sin(r_x);
    float cy = cos(r_y), sy = sin(r_y);
    float cz = cos(r_z), sz = sin(r_z);

    float matrix[3][3] = {
        {
            cy * cz,
            -cy * sz,
            sy
        },
        {
            sx * sy * cz + cx * sz,
            -sx * sy * sz + cx * cz,
            -sx * cy
        },
        {
            -cx * sy * cz + sx * sz,
            cx * sy * sz + sx * cz,
            cx * cy
        }
    };
    
    float temp_x = matrix[0][0] * *x + matrix[0][1] * *y + matrix[0][2] * *z;
    float temp_y = matrix[1][0] * *x + matrix[1][1] * *y + matrix[1][2] * *z;
    float temp_z = matrix[2][0] * *x + matrix[2][1] * *y + matrix[2][2] * *z;

    *x = temp_x;
    *y = temp_y;
    *z = temp_z;
};

__kernel void transformer_kernel(__global Object* objects, __global float* verticies, __global float* normals, __global int* triangles, __global float* transformed_verticies, __global float* transformed_normals, float fov, int width, int height, __global uint* frame_buffer, int ri_h_width, int ri_h_height){ 
    frame_buffer[sizeof(Object)] = 0xFFFFFFFF;int id_x = get_global_id(0);
    
    float vertical_fov_factor = height / tan(0.5 * fov);
    float horizontal_fov_factor = width / tan(0.5 * fov);
    
    int has_normals = 1;
    int base = id_x;


float object_x =   objects[base].transform.position.x;
float object_y =   objects[base].transform.position.y;
float object_z =   objects[base].transform.position.z;
float object_r_x = objects[base].transform.rotation.w;
float object_r_y = objects[base].transform.rotation.x;
float object_r_z = objects[base].transform.rotation.y;
float object_r_w = objects[base].transform.rotation.z;
float object_s_x = objects[base].transform.scale.x; 
float object_s_y = objects[base].transform.scale.y; 
float object_s_z = objects[base].transform.scale.z; 

int triangle_count = objects[base].modelInfo.triangleCount;
int triangle_index = objects[base].modelInfo.triangleOffset;
int vertex_index =   objects[base].modelInfo.vertexOffset;
int transformed_vertex_index = objects[base].modelInfo.transformedVertexOffset;
int normal_index =   objects[base].modelInfo.normalOffset;
int transformed_normal_index =   objects[base].modelInfo.transformedNormalOffset;

for (int triangle = 0; triangle < triangle_count; triangle++){
int triangle_base = (triangle + triangle_index) * 9; 

int i0 = (vertex_index + triangles[triangle_base + 0]) * 3;
int i1 = (vertex_index + triangles[triangle_base + 1]) * 3;
int i2 = (vertex_index + triangles[triangle_base + 2]) * 3;

int i3 = (normal_index + triangles[triangle_base + 3]) * 3;
int i4 = (normal_index + triangles[triangle_base + 4]) * 3;
int i5 = (normal_index + triangles[triangle_base + 5]) * 3;

float x0 = verticies[i0 + 0] * object_s_x;
float y0 = verticies[i0 + 1] * object_s_y;
float z0 = verticies[i0 + 2] * object_s_z;
float x1 = verticies[i1 + 0] * object_s_x;
float y1 = verticies[i1 + 1] * object_s_y;
float z1 = verticies[i1 + 2] * object_s_z;
float x2 = verticies[i2 + 0] * object_s_x;
float y2 = verticies[i2 + 1] * object_s_y;
float z2 = verticies[i2 + 2] * object_s_z;
float n_x0 = normals[i3 + 0];
float n_y0 = normals[i3 + 1];
float n_z0 = normals[i3 + 2];
float n_x1 = normals[i4 + 0];
float n_y1 = normals[i4 + 1];
float n_z1 = normals[i4 + 2];
float n_x2 = normals[i5 + 0];
float n_y2 = normals[i5 + 1];
float n_z2 = normals[i5 + 2];

if (i3 < 0 || i4 < 0 || i5 < 0){
 //   has_normals = 0;
}

if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){
continue;
}

rotate_euler(&x0, &y0, &z0, object_r_x, object_r_y, object_r_z);
rotate_euler(&x1, &y1, &z1, object_r_x, object_r_y, object_r_z);
rotate_euler(&x2, &y2, &z2, object_r_x, object_r_y, object_r_z);

rotate_euler(&n_x0, &n_y0, &n_z0, object_r_x, object_r_y, object_r_z);
rotate_euler(&n_x1, &n_y1, &n_z1, object_r_x, object_r_y, object_r_z);
rotate_euler(&n_x2, &n_y2, &n_z2, object_r_x, object_r_y, object_r_z);

z0 = (z0 + object_z);
x0 = (x0 + object_x) / z0 * horizontal_fov_factor;
y0 = (y0 + object_y) / z0 * vertical_fov_factor;
z1 = (z1 + object_z);
x1 = (x1 + object_x) / z1 * horizontal_fov_factor;
y1 = (y1 + object_y) / z1 * vertical_fov_factor;
z2 = (z2 + object_z);
y2 = (y2 + object_y) / z2 * horizontal_fov_factor;
x2 = (x2 + object_x) / z2 * vertical_fov_factor;

// if ((x0 < -ri_h_width && x1 < -ri_h_width && x2 < -ri_h_width) || (y0 < -ri_h_height && y1 < -ri_h_height && y2 < -ri_h_height) || (x0 >= ri_h_width && x1 >= ri_h_width && x2 >= ri_h_width) || (y0 >= ri_h_height && y1 >= ri_h_height && y2 >= ri_h_height)){
    // transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 0] = 999999;
// }
// else{
    transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 0] = x0;
    transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 1] = y0;
    transformed_verticies[(triangles[triangle_base + 0] + transformed_vertex_index) * 3 + 2] = z0;
    transformed_verticies[(triangles[triangle_base + 1] + transformed_vertex_index) * 3 + 0] = x1;
    transformed_verticies[(triangles[triangle_base + 1] + transformed_vertex_index) * 3 + 1] = y1;
    transformed_verticies[(triangles[triangle_base + 1] + transformed_vertex_index) * 3 + 2] = z1;
    transformed_verticies[(triangles[triangle_base + 2] + transformed_vertex_index) * 3 + 0] = x2;
    transformed_verticies[(triangles[triangle_base + 2] + transformed_vertex_index) * 3 + 1] = y2;
    transformed_verticies[(triangles[triangle_base + 2] + transformed_vertex_index) * 3 + 2] = z2;
    
    transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 0] = n_x0;
    transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 1] = n_y0;
    transformed_normals[(triangles[triangle_base + 0] + transformed_normal_index) * 3 + 2] = n_z0;
    transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 0] = n_x1;
    transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 1] = n_y1;
    transformed_normals[(triangles[triangle_base + 1] + transformed_normal_index) * 3 + 2] = n_z1;
    transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 0] = n_x2;
    transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 1] = n_y2;
    transformed_normals[(triangles[triangle_base + 2] + transformed_normal_index) * 3 + 2] = n_z2;
// }
}}