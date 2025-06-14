static inline float minnn(float a, float b) {
    return (a < b) ? a : b;
}

static inline float maxxx(float a, float b) {
    return (a > b) ? a : b;
}

float clamppp_float(float x, float lower, float upper) {
    return x < lower ? lower : (x > upper ? upper : x);
}

typedef enum {
    RI_MATERIAL_UNLIT = ((ulong)1 << 0), // should calculate lighting
    RI_MATERIAL_DONT_CAST_SHADOW = ((ulong)1 << 1), // should cast shadow on other objects
    RI_MATERIAL_HAS_TEXTURE = ((ulong)1 << 2), // has a texture
    RI_MATERIAL_HAS_NORMAL_MAP = ((ulong)1 << 3), // has a normal map
    RI_MATERIAL_HAS_BUMP_MAP = ((ulong)1 << 4), // has a bump map
    RI_MATERIAL_TRANSPARENT = ((ulong)1 << 5), // has transparency
    RI_MATERIAL_WIREFRAME = ((ulong)1 << 6), // render as wireframe
    RI_MATERIAL_DONT_RECEIVE_SHADOW = ((ulong)1 << 7), // should shadows render on it
    RI_MATERIAL_DONT_DEPTH_TEST = ((ulong)1 << 8), // should check Z buffer (if 1, render on top of everything)
    RI_MATERIAL_DONT_DEPTH_WRITE = ((ulong)1 << 9), // should write to the Z buffer (if 1, render behind everything)
    RI_MATERIAL_DOUBLE_SIDED = ((ulong)1 << 10), // ignore backface culling
} RI_material_properties_enum;

typedef struct {
    float x;
    float y;
} Vec2;

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

int is_intersecting(float a, float b, float c, float d, float p, float q, float r, float s) { 
    float det, gamma, lambda; 
    
    det = (c - a) * (s - q) - (r - p) * (d - b); 
    
    if (det == 0) { 
        return 1; 
    }  
    else { 
        lambda = ((s - q) * (r - a) + (p - r) * (s - b)) / det; 
        gamma = ((b - d) * (r - a) + (c - a) * (s - b)) / det; 
        return (0 < lambda && lambda < 1) && (0 < gamma && gamma < 1); 
    } 
} 

void norm(float dest[2], float a[2]){ 
    float magnitude = sqrt(a[0] * a[0] + a[1] * a[1]); 
    
    dest[0] = a[0] / magnitude; 
    dest[1] = a[1] / magnitude; 
    } 
    
    void sub(float dest[2], float a[2], float b[2]){ 
    dest[0] = a[0] - b[0]; 
    dest[1] = a[1] - b[1]; 
    } 
    
    void add(float dest[2], float a[2], float b[2]){ 
    dest[0] = a[0] + b[0]; 
    dest[1] = a[1] + b[1]; 
} 

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

Vec2 lerp(Vec2 pos1, Vec2 pos2, float fraction){
    Vec2 result;

    float w0 = 1.0 - fraction;
    float w1 = fraction;

    result.x = pos1.x * w0 + pos2.x * w1;
    result.y = pos1.y * w0 + pos2.y * w1;

    return result;

}

__kernel void raster_kernel(__global Object* objects, __global float* transformed_verticies, __global float* transformed_normals, __global float* uvs, __global int* triangles, __global uint* frame_buffer, __global uchar* textures, __global int* texture_info, int object_count, int ri_width, int ri_height, int show_buffer, int frame, float fov, int total_triangle_count, int total_vertex_count, int h_width, int h_height, float wireframe_width){ 
    frame_buffer[sizeof(Object)] = 0xFF00FFFF;int id_x = get_global_id(0) - h_width; 
    int id_y = get_global_id(1) - h_height; 
    
    float z_pixel = INFINITY; 
                    unsigned int frame_pixel = 0x22222222; 
                    
                    float highest_z = 800;
                    float lowest_z = 0;
                    
                    int has_normals = 1;
                    int has_uvs = 1;
                    
                    float w0;
                    float w1;
                    float w2;
                    

                    for (int i_object = 0; i_object < object_count; i_object++){ 
                        int base = i_object;

                        ulong material_flags = objects[base].material.properties;
                        ColorARGB albedo = objects[base].material.albedo;

                        int triangle_count = objects[base].modelInfo.triangleCount;
                        int triangle_index = objects[base].modelInfo.triangleOffset;
                        int vertex_index =   objects[base].modelInfo.vertexOffset;
                        int normal_index =   objects[base].modelInfo.normalOffset;
                        int uv_index =       objects[base].modelInfo.uvOffset;
                        int texture_index =  objects[base].material.textureOffset;
                        int transformed_vertex_index = objects[base].modelInfo.transformedVertexOffset;
                        int transformed_normal_index =   objects[base].modelInfo.transformedNormalOffset;
                        
                        float near_clip = 90;

                        for (int i_triangle = 0; i_triangle < triangle_count; i_triangle++){
                            int triangle_base = (i_triangle + triangle_index) * 9; 

                            int i0 = (transformed_vertex_index + triangles[triangle_base + 0]) * 3;
                            int i1 = (transformed_vertex_index + triangles[triangle_base + 1]) * 3;
                            int i2 = (transformed_vertex_index + triangles[triangle_base + 2]) * 3;

                            int i3 = (transformed_normal_index + triangles[triangle_base + 3]) * 3;
                            int i4 = (transformed_normal_index + triangles[triangle_base + 4]) * 3;
                            int i5 = (transformed_normal_index + triangles[triangle_base + 5]) * 3;

                            int i6 = (uv_index + triangles[triangle_base + 6]) * 3;
                            int i7 = (uv_index + triangles[triangle_base + 7]) * 3;
                            int i8 = (uv_index + triangles[triangle_base + 8]) * 3;
                            
                            float x0 = transformed_verticies[i0 + 0];
                            
                            float z0 = transformed_verticies[i0 + 2];   
                            float z1 = transformed_verticies[i1 + 2];
                            float z2 = transformed_verticies[i2 + 2];
                            
                            float y0 = transformed_verticies[i0 + 1];
                            float x1 = transformed_verticies[i1 + 0];
                            float y1 = transformed_verticies[i1 + 1];
                            float y2 = transformed_verticies[i2 + 1];
                            float x2 = transformed_verticies[i2 + 0]; 
                            int clip_z0 = z0 <= near_clip;
                            int clip_z1 = z1 <= near_clip;
                            int clip_z2 = z2 <= near_clip;
                            
                            int clip_count = clip_z0 + clip_z1 + clip_z2;

                            switch (clip_count){
                                case 0:{ // do nothing, they are all okay >w<
                                    break;}
                                
                                case 1:{ // split triangle into 2
continue;
                                    if (clip_z0){

                                    }

                                    break;
                                }

                                case 2:{ // shrink triangle into 1
                                    if (!clip_z0){ // z0 is fine
                                        float percent_clipped_0 = (near_clip - z0) / (z1 - z0);
                                        float percent_clipped_1 = (near_clip - z0) / (z2 - z0);
                                        
                                        Vec2 pos1 = {x0, y0};
                                        Vec2 pos2 = {x1, y1};

                                        Vec2 p1 = lerp(pos1, pos2, percent_clipped_0);

                                        pos1.x = x2;
                                        pos1.y = y2;

                                        Vec2 p2 = lerp(pos1, pos2, percent_clipped_1);

                                        x1 = p1.x;
                                        y1 = p1.y;

                                        x2 = p2.x;
                                        y2 = p2.y;

                                        z1 = near_clip;
                                        z2 = near_clip;

                                        break;
                                    }
                                    
                                    if (!clip_z1){
                                        float percent_clipped_0 = (near_clip - z1) / (z0 - z1);
                                        float percent_clipped_1 = (near_clip - z1) / (z2 - z1);
                                        
                                        Vec2 pos1 = {x1, y1};
                                        Vec2 pos2 = {x0, y0};
                                        
                                        Vec2 p1 = lerp(pos1, pos2, percent_clipped_0);
                                        
                                        pos1.x = x2;
                                        pos1.y = y2;
                                        
                                        Vec2 p2 = lerp(pos1, pos2, percent_clipped_1);
                                        
                                        x0 = p1.x;
                                        y0 = p1.y;
                                        
                                        x2 = p2.x;
                                        y2 = p2.y;
                                        
                                        z0 = near_clip;
                                        z2 = near_clip;
                                        material_flags = 0;
                                        break;
                                    }

                                    if (!clip_z2){
                                        float percent_clipped_0 = (near_clip - z2) / (z1 - z2);
                                        float percent_clipped_1 = (near_clip - z2) / (z0 - z2);
                                        
                                        Vec2 pos1 = {x2, y2};
                                        Vec2 pos2 = {x1, y1};

                                        Vec2 p1 = lerp(pos1, pos2, percent_clipped_0);

                                        pos1.x = x0;
                                        pos1.y = y0;

                                        Vec2 p2 = lerp(pos1, pos2, percent_clipped_1);

                                        x1 = p1.x;
                                        y1 = p1.y;

                                        x0 = p2.x;
                                        y0 = p2.y;

                                        z1 = near_clip;
                                        z0 = near_clip;
                                        break;
                                    }
                                }

                                case 3:{ // lost cause, exit
                                    continue;
                                }

                                default: { // shouldn't happen
                                    continue;
                                }
                            }
                            
                            
                            if (i3 < 0 || i4 < 0 || i5 < 0){
                                has_normals = 0;
                            }
                            if (i6 < 0 || i7 < 0 || i8 < 0){
                                has_uvs = 0;
                            }
                            
                            if (isinf(x0) || isinf(y0) || isinf(z0) || isinf(x1) || isinf(y1) || isinf(z1) || isinf(x2) || isinf(y2) || isinf(z2)){
                                continue;
                            }
                            
                            float smallest_x = x0; 
                            float largest_x = x0; 
                            float smallest_y = y0; 
                            float largest_y = y0; 
                            
                            if (x0 > largest_x) largest_x = x0;
                            if (x1 > largest_x) largest_x = x1;
                            if (x2 > largest_x) largest_x = x2;
                            
                            if (x0 < smallest_x) smallest_x = x0;
                            if (x1 < smallest_x) smallest_x = x1;
                            if (x2 < smallest_x) smallest_x = x2;
                            
                            if (y0 > largest_y) largest_y = y0;
                            if (y1 > largest_y) largest_y = y1;
                            if (y2 > largest_y) largest_y = y2;
                            
                            if (y0 < smallest_y) smallest_y = y0;
                            if (y1 < smallest_y) smallest_y = y1;
                            if (y2 < smallest_y) smallest_y = y2;
                            
                            smallest_x = minnn(smallest_x, 0); 
                            largest_x = maxxx(largest_x, ri_width);  
                            smallest_y = minnn(smallest_y, 0); 
                            largest_y = maxxx(largest_y, ri_height); 
                            
                            if (id_x >= smallest_x && id_x <= largest_x && id_y >= smallest_y && id_y <= largest_y){ 
                                float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2); 
                                
                                if (!(material_flags & RI_MATERIAL_DOUBLE_SIDED) && denominator >= 0) { 
                                    continue; 
                                } 
                                
                                w0 = ((y1 - y2) * (id_x - x2) + (x2 - x1) * (id_y - y2)) / denominator; 
                                w1 = ((y2 - y0) * (id_x - x0) + (x0 - x2) * (id_y - y0)) / denominator; 
                                w2 = 1.0 - w0 - w1; 
                                
                                if (!(w0 > 0 && w1 > 0 && w2 > 0)){
                                    continue;
                                }

                                if (material_flags & RI_MATERIAL_WIREFRAME && (w0 >= wireframe_width && w1 >= wireframe_width && w2 >= wireframe_width)){
                                    continue;
                                }

                                float w_over_z = (w0 / z0 + w1 / z1 + w2 / z2); 
                                float z = 1.0 / w_over_z;

                                if (z < z_pixel){ 
                                    z_pixel = z; 
                                    
                                    float n_x0 = transformed_normals[i3 + 0];
                                    float n_y0 = transformed_normals[i3 + 1];
                                    float n_z0 = transformed_normals[i3 + 2];
                                    
                                    float n_x1 = transformed_normals[i4 + 0];
                                    float n_y1 = transformed_normals[i4 + 1];
                                    float n_z1 = transformed_normals[i4 + 2];
                                    
                                    float n_x2 = transformed_normals[i5 + 0];
                                    float n_y2 = transformed_normals[i5 + 1];
                                    float n_z2 = transformed_normals[i5 + 2];
                                    
                                    float u_x0 = uvs[i6 + 0];
                                    float u_y0 = uvs[i6 + 1];
                                    float u_z0 = uvs[i6 + 2];
                                    
                                    float u_x1 = uvs[i7 + 0];
                                    float u_y1 = uvs[i7 + 1];
                                    float u_z1 = uvs[i7 + 2];
                                    
                                    float u_x2 = uvs[i8 + 0];
                                    float u_y2 = uvs[i8 + 1];
                                    float u_z2 = uvs[i8 + 2];
                                    
                                    switch (show_buffer){
                                        case 0:{
                                            if (!(material_flags & RI_MATERIAL_HAS_TEXTURE)){
                                                frame_pixel = (albedo.a << 24) | (albedo.r << 16) | (albedo.g << 8) | albedo.b;
                                            
                                                break;
                                            }

                                            double ux = (w0 * (u_x0 / z0) + w1 * (u_x1 / z1) + w2 * (u_x2 / z2)) / w_over_z;
                                            double uy = (w0 * (u_y0 / z0) + w1 * (u_y1 / z1) + w2 * (u_y2 / z2)) / w_over_z;
                                            
                                            int texture_width = texture_info[texture_index * 3];
                                            int texture_height = texture_info[texture_index * 3 + 1];
                                            int texture_value_offset = texture_info[texture_index * 3 + 2];
                                            
                                            int ix = maxxx((int)(ux * texture_width), 0);
                                            int iy = maxxx((int)(uy * texture_height), 0);
                                            
                                            int uv_pixel = (iy * texture_width + ix) * 4 + texture_value_offset;

                                            if (uv_pixel >= texture_width * texture_height * 4 + texture_value_offset)break;
                                            
                                            unsigned char r = textures[uv_pixel + 0];
                                            unsigned char g = textures[uv_pixel + 1];
                                            unsigned char b = textures[uv_pixel + 2];
                                            unsigned char a = textures[uv_pixel + 3];
                                            
                                            frame_pixel = (a << 24) | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 1:{
                                            float z = clamppp_float(z_pixel, 0.0f, highest_z);
                                            
                                            float norm_z = z / highest_z;
                                            
                                            unsigned char intensity = (unsigned char)(norm_z * 255.0f);
                                            
                                            frame_pixel = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;
                                            
                                            break;}
                                        case 2:{
                                            float nx = (w0 * (n_x0 / z0) + w1 * (n_x1 / z1) + w2 * (n_x2 / z2)) / w_over_z;
                                            float ny = (w0 * (n_y0 / z0) + w1 * (n_y1 / z1) + w2 * (n_y2 / z2)) / w_over_z;
                                            float nz = (w0 * (n_z0 / z0) + w1 * (n_z1 / z1) + w2 * (n_z2 / z2)) / w_over_z;
                                            
                                            nx = clamppp_float((nx * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            ny = clamppp_float((ny * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            nz = clamppp_float((nz * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
                                            
                                            unsigned char r = (unsigned char)nx;
                                            unsigned char g = (unsigned char)ny;
                                            unsigned char b = (unsigned char)nz;
                                            
                                            if (!has_normals){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        case 3:{
                                            float ux = w0 * u_x0 + w1 * u_x1 + w2 * u_x2;
                                            float uy = w0 * u_y0 + w1 * u_y1 + w2 * u_y2;
                                            
                                            unsigned char r = (unsigned char)clamppp_float(ux * 255.0f, 0.0f, 255.0f);
                                            unsigned char g = (unsigned char)clamppp_float(uy * 255.0f, 0.0f, 255.0f);
                                            unsigned char b = 0;
                                            
                                            if (!has_uvs){
                                                r = 20;
                                                g = 20;
                                                b = 20;
                                            }
                                            
                                            frame_pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                                            
                                            break;}
                                        default:{
                                            frame_pixel = 0xFF00FFFF;
                                            
                                            break;}
                                    }
                                } 
                            }
                        }
                    }
                    
                    int pixel_coord = (ri_height * 0.5 - id_y) * ri_width + id_x + ri_width * 0.5;
                    
                    if (pixel_coord >= ri_width * ri_height || pixel_coord < 0){
                        return;
                    }
                    
                    frame_buffer[pixel_coord] = frame_pixel; 
                }
