#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef struct {
    double w;
    double x;
    double y;
    double z;
} RI_vector_4;
    
typedef struct {
    double x;
    double y;
    double z;
} RI_vector_3;

typedef struct {
    double x;
    double y;
} RI_vector_2;

typedef struct {
    RI_vector_3 position_0, position_1, position_2;
    RI_vector_3 normal_0, normal_1, normal_2;
    RI_vector_2 uv_0, uv_1, uv_2;
    short min_screen_x, max_screen_x, min_screen_y, max_screen_y;
    uchar should_render;
    ushort texture_width, texture_height;
    uint texture_index;
} RI_renderable_face;

typedef struct {
    RI_vector_3 position_0;
    RI_vector_3 position_1;
    RI_vector_3 position_2;

    RI_vector_3 normal_0;
    RI_vector_3 normal_1;
    RI_vector_3 normal_2;

    RI_vector_2 uv_0;
    RI_vector_2 uv_1;
    RI_vector_2 uv_2;

    uchar should_render;
} RI_face;

typedef struct {
    RI_vector_3 position;
    RI_vector_4 rotation;
    RI_vector_3 scale;;
    uchar active;
    uchar has_normals;
    uchar has_uvs;
    ushort material_index;
    uint face_index;
    uint face_count;
} RI_actor;

typedef struct {
    RI_vector_3 position;
    RI_vector_4 rotation;
    float FOV, min_clip, max_clip;
} RI_camera;

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_2_times(RI_vector_2 *vector, double value){
    vector->x *= value;
    vector->y *= value;
}

void global_vector_2_times(__global RI_vector_2 *vector, double value){
    vector->x *= value;
    vector->y *= value;
}

// set all values of a vector 3
void vector_3_memset(RI_vector_3 *vector, double value){
    vector->x = value;
    vector->y = value;
    vector->z = value;
}

// set all values of a vector 2
void vector_2_memset(RI_vector_2 *vector, double value){
    vector->x = value;
    vector->y = value;
}

// value-wise multiplacation.
// multiply the whole vector by 1 value
void vector_3_times(RI_vector_3 *vector, double value){
    vector->x *= value;
    vector->y *= value;
    vector->z *= value;
}

void global_vector_3_times(__global RI_vector_3 *vector, double value){
    vector->x *= value;
    vector->y *= value;
    vector->z *= value;
}

// hadamard multiplacation.
// multiply each value of one vector with the matching one on the other vector
void vector_3_hadamard(RI_vector_3 *multiplicand, RI_vector_3 multiplicator){
    multiplicand->x *= multiplicator.x;
    multiplicand->y *= multiplicator.y;
    multiplicand->z *= multiplicator.z;
}

void global_vector_3_hadamard(__global RI_vector_3 *multiplicand, RI_vector_3 multiplicator){
    multiplicand->x *= multiplicator.x;
    multiplicand->y *= multiplicator.y;
    multiplicand->z *= multiplicator.z;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_2_element_wise_add(RI_vector_2 *addend_a, RI_vector_2 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
}

void global_vector_2_element_wise_add(__global RI_vector_2 *addend_a, RI_vector_2 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
}

// "hadamard" addition.
// add each value of one vector with the matching one on the other vector
void vector_3_element_wise_add(RI_vector_3 *addend_a, RI_vector_3 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

void global_vector_3_element_wise_add(__global RI_vector_3 *addend_a, RI_vector_3 addend_b){
    addend_a->x += addend_b.x;
    addend_a->y += addend_b.y;
    addend_a->z += addend_b.z;
}

// "hadamard" subtraction.
// subtraction each value of one vector with the matching one on the other vector
void vector_3_element_wise_subtract(RI_vector_3 *minuend, RI_vector_3 subtrahend){
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
    minuend->z -= subtrahend.z;
}

void global_vector_3_element_wise_subtract(__global RI_vector_3 *minuend, RI_vector_3 subtrahend){
    minuend->x -= subtrahend.x;
    minuend->y -= subtrahend.y;
    minuend->z -= subtrahend.z;
}

// conjugate a quaterion.
// (flip the sign of the x, y, z values)
void quaternion_conjugate(RI_vector_4* quaternion){
    quaternion->x *= -1.0;
    quaternion->y *= -1.0;
    quaternion->z *= -1.0;
}

void globla_quaternion_conjugate(__global RI_vector_4* quaternion){
    quaternion->x *= -1.0;
    quaternion->y *= -1.0;
    quaternion->z *= -1.0;
}

// quaternion multiplacation
void quaternion_multiply(RI_vector_4* a, RI_vector_4 b){
    double w1 = a->w; double x1 = a->x; double y1 = a->y; double z1 = a->z;
    double w2 = b.w; double x2 = b.x; double y2 = b.y; double z2 = b.z;

    double w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    double x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    double y = w1*y2 - x1*z2 + y1*w2 + z1*x2;
    double z = w1*z2 + x1*y2 - y1*x2 + z1*w2;

    *a = (RI_vector_4){w, x, y, z};
}

void global_quaternion_multiply(__global RI_vector_4* a, RI_vector_4 b){
    double w1 = a->w; double x1 = a->x; double y1 = a->y; double z1 = a->z;
    double w2 = b.w; double x2 = b.x; double y2 = b.y; double z2 = b.z;

    double w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    double x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    double y = w1*y2 - x1*z2 + y1*w2 + z1*x2;
    double z = w1*z2 + x1*y2 - y1*x2 + z1*w2;

    *a = (RI_vector_4){w, x, y, z};
}

// linear interpolate between 2 vectors
void vector_2_lerp(RI_vector_2 vector_a, RI_vector_2 vector_b, RI_vector_2 *result, double w1){
    double w0 = 1.0 - w1;

    vector_2_memset(result, 0);

    vector_2_times(&vector_a, w0);
    vector_2_times(&vector_b, w1);

    vector_2_element_wise_add(result, vector_a);
    vector_2_element_wise_add(result, vector_b);
}

void global_vector_2_lerp(RI_vector_2 vector_a, RI_vector_2 vector_b, __global RI_vector_2 *result, double w1){
    double w0 = 1.0 - w1;

    global_vector_2_times(result, 0);

    vector_2_times(&vector_a, w0);
    vector_2_times(&vector_b, w1);

    global_vector_2_element_wise_add(result, vector_a);
    global_vector_2_element_wise_add(result, vector_b);
}

// linear interpolate between 2 vectors
void vector_3_lerp(RI_vector_3 vector_a, RI_vector_3 vector_b, RI_vector_3 *result, double w1){
    double w0 = 1.0 - w1;

    vector_3_memset(result, 0);

    vector_3_times(&vector_a, w0);
    vector_3_times(&vector_b, w1);

    vector_3_element_wise_add(result, vector_a);
    vector_3_element_wise_add(result, vector_b);
}

void global_vector_3_lerp(RI_vector_3 vector_a, RI_vector_3 vector_b, __global RI_vector_3 *result, double w1){
    double w0 = 1.0 - w1;

    global_vector_3_times(result, 0);

    vector_3_times(&vector_a, w0);
    vector_3_times(&vector_b, w1);

    global_vector_3_element_wise_add(result, vector_a);
    global_vector_3_element_wise_add(result, vector_b);
}

void quaternion_rotate(RI_vector_3 *position, RI_vector_4 rotation){
    RI_vector_4 pos_quat = {0, position->x, position->y, position->z};

    RI_vector_4 rotation_conjugation = rotation;
    quaternion_conjugate(&rotation_conjugation);

    quaternion_multiply(&rotation, pos_quat);

    quaternion_multiply(&rotation, rotation_conjugation);


    *position = (RI_vector_3){rotation.x, rotation.y, rotation.z};
}

void global_quaternion_rotate(__global RI_vector_3 *position, RI_vector_4 rotation){
    RI_vector_4 pos_quat = {0, position->x, position->y, position->z};

    RI_vector_4 rotation_conjugation = rotation;
    quaternion_conjugate(&rotation_conjugation);

    quaternion_multiply(&rotation, pos_quat);

    quaternion_multiply(&rotation, rotation_conjugation);

    *position = (RI_vector_3){rotation.x, rotation.y, rotation.z};
}

__kernel void clear_tile_array(__global uint* tiles){
    uint num_faces_per_tile = tiles[2];
    uint number_of_horizontal_tiles = tiles[3];
    uint number_of_vertical_tiles = tiles[4];

    int x = get_global_id(0); if (x >= number_of_horizontal_tiles) return;
    int y = get_global_id(1); if (y >= number_of_vertical_tiles) return;

    uint index = 5 + (y * number_of_horizontal_tiles + x) * (1 + num_faces_per_tile); 

    tiles[index] = 0;

    return;
}

__kernel void transformer(__global RI_face *faces, __global RI_renderable_face *renderable_faces, double actor_x, double actor_y, double actor_z, double actor_r_w, double actor_r_x, double actor_r_y, double actor_r_z, double actor_s_x, double actor_s_y, double actor_s_z, int has_normals, int has_uvs, int face_array_offset_index, int face_count, int width, int height, double horizontal_fov_factor, double vertical_fov_factor, float min_clip, float max_clip, double camera_x, double camera_y, double camera_z, double camera_r_w, double camera_r_x, double camera_r_y, double camera_r_z, int renderable_face_offset, int face_sqrt, ushort texture_width, ushort texture_height, uint texture_index, __global uint* tiles){
    int face_index = get_global_id(1) * face_sqrt + get_global_id(0); if (face_index >= face_count) return;

    RI_vector_3 current_actor_position = (RI_vector_3){actor_x, actor_y, actor_z};
    RI_vector_4 current_actor_rotation = (RI_vector_4){actor_r_w, actor_r_x, actor_r_y, actor_r_z};
    RI_vector_3 current_actor_scale = (RI_vector_3){actor_s_x, actor_s_y, actor_s_z};
    RI_vector_3 camera_position = (RI_vector_3){camera_x, camera_y, camera_z};
    RI_vector_4 camera_rotation = (RI_vector_4){camera_r_w, camera_r_x, camera_r_y, camera_r_z};

    __global RI_face *cur_face = &faces[face_index + face_array_offset_index];

    __global RI_renderable_face *cur_r_face = &renderable_faces[face_index * 2 + renderable_face_offset];

    // set split face not to render
    renderable_faces[face_index * 2 + renderable_face_offset + 1].should_render = 0;

    cur_r_face->should_render = 1;

    // cur_r_face->material = current_actor.material;

    cur_r_face->position_0 = cur_face->position_0;
    cur_r_face->position_1 = cur_face->position_1;
    cur_r_face->position_2 = cur_face->position_2;

    cur_r_face->normal_0 = cur_face->normal_0;
    cur_r_face->normal_1 = cur_face->normal_1;
    cur_r_face->normal_2 = cur_face->normal_2;

    if (has_uvs){
        cur_r_face->uv_0 = cur_face->uv_0;
        cur_r_face->uv_1 = cur_face->uv_1;
        cur_r_face->uv_2 = cur_face->uv_2;
    }

    cur_r_face->texture_width = texture_width;
    cur_r_face->texture_height = texture_height;
    cur_r_face->texture_index = texture_index;

    // scale
    global_vector_3_hadamard(&cur_r_face->position_0, current_actor_scale);
    global_vector_3_hadamard(&cur_r_face->position_1, current_actor_scale);
    global_vector_3_hadamard(&cur_r_face->position_2, current_actor_scale);

    // actor rotation
    global_quaternion_rotate(&cur_r_face->position_0, current_actor_rotation);
    global_quaternion_rotate(&cur_r_face->position_1, current_actor_rotation);
    global_quaternion_rotate(&cur_r_face->position_2, current_actor_rotation);

    global_quaternion_rotate(&cur_r_face->normal_0, current_actor_rotation);
    global_quaternion_rotate(&cur_r_face->normal_1, current_actor_rotation);
    global_quaternion_rotate(&cur_r_face->normal_2, current_actor_rotation);
    
    // object position
    global_vector_3_element_wise_add(&cur_r_face->position_0, current_actor_position);
    global_vector_3_element_wise_add(&cur_r_face->position_1, current_actor_position);
    global_vector_3_element_wise_add(&cur_r_face->position_2, current_actor_position);    

    // camera position & rotation
    global_vector_3_element_wise_subtract(&cur_r_face->position_0, camera_position);
    global_vector_3_element_wise_subtract(&cur_r_face->position_1, camera_position);
    global_vector_3_element_wise_subtract(&cur_r_face->position_2, camera_position);

    global_quaternion_rotate(&cur_r_face->position_0, camera_rotation);
    global_quaternion_rotate(&cur_r_face->position_1, camera_rotation);
    global_quaternion_rotate(&cur_r_face->position_2, camera_rotation);        

    __global RI_vector_3 *pos_0 = &cur_r_face->position_0;
    __global RI_vector_3 *pos_1 = &cur_r_face->position_1;
    __global RI_vector_3 *pos_2 = &cur_r_face->position_2;

    int is_0_clipped = pos_0->z < min_clip;
    int is_1_clipped = pos_1->z < min_clip;
    int is_2_clipped = pos_2->z < min_clip;

    int clip_count = is_0_clipped + is_1_clipped + is_2_clipped;

    cur_r_face->should_render = 1;

    switch(clip_count){
        case 3: {// ignore polygon, it's behind the camera
            return;
            break;
        }

        case 2:{ // shrink poylgon
            __global RI_vector_3 *unclipped_point, *point_a, *point_b;
            __global RI_vector_3 *unclipped_normal, *normal_a, *normal_b;
            __global RI_vector_2 *unclipped_uv, *uv_a, *uv_b;

            if (!is_0_clipped){ 
                unclipped_point = &cur_r_face->position_0;
                point_a = &cur_r_face->position_1;
                point_b = &cur_r_face->position_2;
                
                unclipped_normal = &cur_r_face->normal_0;
                normal_a = &cur_r_face->normal_1;
                normal_b = &cur_r_face->normal_2;
            
                unclipped_uv = &cur_r_face->uv_0;
                uv_a = &cur_r_face->uv_1;
                uv_b = &cur_r_face->uv_2;
            }
            else if (!is_1_clipped){ 
                unclipped_point = &cur_r_face->position_1;
                point_a = &cur_r_face->position_2;
                point_b = &cur_r_face->position_0;
                
                unclipped_normal = &cur_r_face->normal_1;
                normal_a = &cur_r_face->normal_2;
                normal_b = &cur_r_face->normal_0;
            
                unclipped_uv = &cur_r_face->uv_1;
                uv_a = &cur_r_face->uv_2;
                uv_b = &cur_r_face->uv_0;
            }
            else if (!is_2_clipped){ 
                unclipped_point = &cur_r_face->position_2;
                point_a = &cur_r_face->position_0;
                point_b = &cur_r_face->position_1;
                
                unclipped_normal = &cur_r_face->normal_2;
                normal_a = &cur_r_face->normal_0;
                normal_b = &cur_r_face->normal_1;
            
                unclipped_uv = &cur_r_face->uv_2;
                uv_a = &cur_r_face->uv_0;
                uv_b = &cur_r_face->uv_1;
            }
        
            double fraction_a_to_unclip = (min_clip - unclipped_point->z) / (point_a->z - unclipped_point->z);                          
            double fraction_b_to_unclip = (min_clip - unclipped_point->z) / (point_b->z - unclipped_point->z);  

            global_vector_3_lerp(*unclipped_point, *point_a, point_a, fraction_a_to_unclip);
            global_vector_3_lerp(*unclipped_point, *point_b, point_b, fraction_b_to_unclip);

            global_vector_3_lerp(*unclipped_normal, *normal_a, normal_a, fraction_a_to_unclip);
            global_vector_3_lerp(*unclipped_normal, *normal_b, normal_b, fraction_b_to_unclip);

            global_vector_2_lerp(*unclipped_uv, *uv_a, uv_a, fraction_a_to_unclip);
            global_vector_2_lerp(*unclipped_uv, *uv_b, uv_b, fraction_b_to_unclip);

            break;}

        case 1: {// split polygon
            RI_vector_3 clipped_point, point_a, point_b;
            RI_vector_3 clipped_normal, normal_a, normal_b;
            RI_vector_2 clipped_uv, uv_a, uv_b;

            if (is_0_clipped){ 
                clipped_point = cur_r_face->position_0;
                point_a = cur_r_face->position_1;
                point_b = cur_r_face->position_2;
                
                clipped_normal = cur_r_face->normal_0;
                normal_a = cur_r_face->normal_1;
                normal_b = cur_r_face->normal_2;
            
                clipped_uv = cur_r_face->uv_0;
                uv_a = cur_r_face->uv_1;
                uv_b = cur_r_face->uv_2;
            }
            else if (is_1_clipped){ 
                clipped_point = cur_r_face->position_1;
                point_a = cur_r_face->position_2;
                point_b = cur_r_face->position_0;
                
                clipped_normal = cur_r_face->normal_1;
                normal_a = cur_r_face->normal_2;
                normal_b = cur_r_face->normal_0;
            
                clipped_uv = cur_r_face->uv_1;
                uv_a = cur_r_face->uv_2;
                uv_b = cur_r_face->uv_0;
            }
            else if (is_2_clipped){ 
                clipped_point = cur_r_face->position_2;
                point_a = cur_r_face->position_0;
                point_b = cur_r_face->position_1;
                
                clipped_normal = cur_r_face->normal_2;
                normal_a = cur_r_face->normal_0;
                normal_b = cur_r_face->normal_1;
            
                clipped_uv = cur_r_face->uv_2;
                uv_a = cur_r_face->uv_0;
                uv_b = cur_r_face->uv_1;
            }

            double fraction_a_to_clip = (min_clip - clipped_point.z) / (point_a.z - clipped_point.z);                        
            double fraction_b_to_clip = (min_clip - clipped_point.z) / (point_b.z - clipped_point.z);                        

            RI_vector_3 new_point_a, new_point_b;  // the new points that move along the polygon's edge to match the z value of min_clip.
            RI_vector_3 new_normal_a, new_normal_b;  // they come from the clipped point which was originally only 1
            RI_vector_2 new_uv_a, new_uv_b;

            vector_3_lerp(clipped_point, point_a, &new_point_a, fraction_a_to_clip);
            vector_3_lerp(clipped_point, point_b, &new_point_b, fraction_b_to_clip);
            
            vector_3_lerp(clipped_normal, normal_a, &new_normal_a, fraction_a_to_clip);
            vector_3_lerp(clipped_normal, normal_b, &new_normal_b, fraction_b_to_clip);
            
            vector_2_lerp(clipped_uv, uv_a, &new_uv_a, fraction_a_to_clip);
            vector_2_lerp(clipped_uv, uv_b, &new_uv_b, fraction_b_to_clip);

            // okay, now we have a quad (in clockwise order, point a, point b, new point b, new point a)
            // quads are easy to turn into tris >w<

            __global RI_renderable_face *cur_r_split_face = &renderable_faces[face_index * 2 + renderable_face_offset + 1];

            // cur_r_split_face->parent_actor = current_actor;

            // cur_r_split_face->material = cur_r_face->material;


            cur_r_face->position_0 = point_a;
            cur_r_face->position_1 = point_b;
            cur_r_face->position_2 = new_point_a;

            cur_r_face->normal_0 = normal_a;
            cur_r_face->normal_1 = normal_b;
            cur_r_face->normal_2 = new_normal_a;

            cur_r_face->uv_0 = uv_a;
            cur_r_face->uv_1 = uv_b;
            cur_r_face->uv_2 = new_uv_a;

            cur_r_split_face->position_0 = point_b;
            cur_r_split_face->position_1 = new_point_b;
            cur_r_split_face->position_2 = new_point_a;

            cur_r_split_face->normal_0 = normal_b;
            cur_r_split_face->normal_1 = new_normal_b;
            cur_r_split_face->normal_2 = new_normal_a;

            cur_r_split_face->uv_0 = uv_b;
            cur_r_split_face->uv_1 = new_uv_b;
            cur_r_split_face->uv_2 = new_uv_a;

            cur_r_split_face->position_0.x = cur_r_split_face->position_0.x / cur_r_split_face->position_0.z * horizontal_fov_factor;
            cur_r_split_face->position_0.y = cur_r_split_face->position_0.y / cur_r_split_face->position_0.z * vertical_fov_factor;
            
            cur_r_split_face->position_1.x = cur_r_split_face->position_1.x / cur_r_split_face->position_1.z * horizontal_fov_factor;
            cur_r_split_face->position_1.y = cur_r_split_face->position_1.y / cur_r_split_face->position_1.z * vertical_fov_factor;

            cur_r_split_face->position_2.x = cur_r_split_face->position_2.x / cur_r_split_face->position_2.z * horizontal_fov_factor;
            cur_r_split_face->position_2.y = cur_r_split_face->position_2.y / cur_r_split_face->position_2.z * vertical_fov_factor;

            cur_r_split_face->min_screen_x = cur_r_split_face->position_0.x; 
            if (cur_r_split_face->position_1.x < cur_r_split_face->min_screen_x) cur_r_split_face->min_screen_x = cur_r_split_face->position_1.x;
            if (cur_r_split_face->position_2.x < cur_r_split_face->min_screen_x) cur_r_split_face->min_screen_x = cur_r_split_face->position_2.x;
            cur_r_split_face->min_screen_x = max(cur_r_split_face->min_screen_x, (short)(-width / 2)); 

            cur_r_split_face->max_screen_x = cur_r_split_face->position_0.x; 
            if (cur_r_split_face->position_1.x > cur_r_split_face->max_screen_x) cur_r_split_face->max_screen_x = cur_r_split_face->position_1.x;
            if (cur_r_split_face->position_2.x > cur_r_split_face->max_screen_x) cur_r_split_face->max_screen_x = cur_r_split_face->position_2.x;
            cur_r_split_face->max_screen_x = min(cur_r_split_face->max_screen_x, (short)(width / 2)); 

            cur_r_split_face->min_screen_y = cur_r_split_face->position_0.y; 
            if (cur_r_split_face->position_1.y < cur_r_split_face->min_screen_y) cur_r_split_face->min_screen_y = cur_r_split_face->position_1.y;
            if (cur_r_split_face->position_2.y < cur_r_split_face->min_screen_y) cur_r_split_face->min_screen_y = cur_r_split_face->position_2.y;
            cur_r_split_face->min_screen_y = max(cur_r_split_face->min_screen_y, (short)(-height / 2)); 

            cur_r_split_face->max_screen_y = cur_r_split_face->position_0.y; 
            if (cur_r_split_face->position_1.y > cur_r_split_face->max_screen_y) cur_r_split_face->max_screen_y = cur_r_split_face->position_1.y;
            if (cur_r_split_face->position_2.y > cur_r_split_face->max_screen_y) cur_r_split_face->max_screen_y = cur_r_split_face->position_2.y;
            cur_r_split_face->max_screen_y = min(cur_r_split_face->max_screen_y, (short)(height / 2)); 

            cur_r_split_face->should_render = 1;

            cur_r_split_face->texture_width = texture_width;
            cur_r_split_face->texture_height = texture_height;
            cur_r_split_face->texture_index = texture_index;

            break;
        }

        case 0:{ // no issues, ignore
            break;
        }
    }

    // current_actor.material->vertex_shader(&cur_r_face->position_0, &cur_r_face->position_1, &cur_r_face->position_2, horizontal_fov_factor, vertical_fov_factor);

    cur_r_face->position_0.x = cur_r_face->position_0.x / cur_r_face->position_0.z * horizontal_fov_factor;
    cur_r_face->position_0.y = cur_r_face->position_0.y / cur_r_face->position_0.z * vertical_fov_factor;
    
    cur_r_face->position_1.x = cur_r_face->position_1.x / cur_r_face->position_1.z * horizontal_fov_factor;
    cur_r_face->position_1.y = cur_r_face->position_1.y / cur_r_face->position_1.z * vertical_fov_factor;

    cur_r_face->position_2.x = cur_r_face->position_2.x / cur_r_face->position_2.z * horizontal_fov_factor;
    cur_r_face->position_2.y = cur_r_face->position_2.y / cur_r_face->position_2.z * vertical_fov_factor;

    cur_r_face->min_screen_x = pos_0->x; 
    if (pos_1->x < cur_r_face->min_screen_x) cur_r_face->min_screen_x = pos_1->x;
    if (pos_2->x < cur_r_face->min_screen_x) cur_r_face->min_screen_x = pos_2->x;
    cur_r_face->min_screen_x = max(cur_r_face->min_screen_x, (short)(-width / 2)); 

    cur_r_face->max_screen_x = pos_0->x; 
    if (pos_1->x > cur_r_face->max_screen_x) cur_r_face->max_screen_x = pos_1->x;
    if (pos_2->x > cur_r_face->max_screen_x) cur_r_face->max_screen_x = pos_2->x;
    cur_r_face->max_screen_x = min(cur_r_face->max_screen_x, (short)(width / 2)); 

    cur_r_face->min_screen_y = pos_0->y; 
    if (pos_1->y < cur_r_face->min_screen_y) cur_r_face->min_screen_y = pos_1->y;
    if (pos_2->y < cur_r_face->min_screen_y) cur_r_face->min_screen_y = pos_2->y;
    cur_r_face->min_screen_y = max(cur_r_face->min_screen_y, (short)(-height / 2)); 

    cur_r_face->max_screen_y = pos_0->y; 
    if (pos_1->y > cur_r_face->max_screen_y) cur_r_face->max_screen_y = pos_1->y;
    if (pos_2->y > cur_r_face->max_screen_y) cur_r_face->max_screen_y = pos_2->y;
    cur_r_face->max_screen_y = min(cur_r_face->max_screen_y, (short)(height / 2)); 

    uint tile_width = tiles[0];
    uint tile_height = tiles[1];
    uint faces_per_tile = tiles[2];
    uint number_of_horizontal_tiles = tiles[3];
    uint number_of_vertical_tiles = tiles[4];

    uint tile_x_min = fmin(fmax(floor((float)((float)(cur_r_face->min_screen_x + width / 2) / (float)tile_width)), 0), number_of_horizontal_tiles - 1);
    uint tile_x_max = fmin(fmax(floor((float)((float)(cur_r_face->max_screen_x + width / 2) / (float)tile_width)), 0), number_of_horizontal_tiles - 1);    
    uint tile_y_min = fmin(fmax(floor((float)((float)(cur_r_face->min_screen_y + height / 2) / (float)tile_height)), 0), number_of_vertical_tiles - 1);
    uint tile_y_max = fmin(fmax(floor((float)((float)(cur_r_face->max_screen_y + height / 2) / (float)tile_height)), 0), number_of_vertical_tiles - 1);

    for (uint y = tile_y_min; y <= tile_y_max; y++){
        for (uint x = tile_x_min; x <= tile_x_max; x++){
            uint tile_array_index = 5 + (y * number_of_horizontal_tiles + x) * (1 + faces_per_tile);

            uint num_faces_in_cur_tile = atomic_fetch_add((volatile __global atomic_uint*)&tiles[tile_array_index], 1);

            tiles[tile_array_index + num_faces_in_cur_tile + 1] = face_index * 2 + renderable_face_offset;
        }
    }

    return;
}

__kernel void rasterizer(__global RI_renderable_face *renderable_faces, __global uint* textures, __global uint *frame_buffer, int width, int height, int half_width, int half_height, int number_of_renderable_faces, int number_of_split_renderable_faces, __global uint* tiles){
    int pixel_x = get_global_id(0); if (pixel_x >= width) return;
    int pixel_y = get_global_id(1); if (pixel_y >= height) return;
    int idx = (height - pixel_y) * width + pixel_x;

    int x = pixel_x - half_width;
    int y = pixel_y - half_height;

    uint tile_width = tiles[0];
    uint tile_height = tiles[1];
    uint faces_per_tile = tiles[2];
    uint number_of_horizontal_tiles = tiles[3];
    uint number_of_vertical_tiles = tiles[4];

    uint tile_x = fmin(fmax(floor((float)(pixel_x / tile_width)), 0), number_of_horizontal_tiles);
    uint tile_y = fmin(fmax(floor((float)(pixel_y / tile_height)), 0), number_of_vertical_tiles);

    double z = INFINITY;

    uint pixel_color = 0x11111111;

    uint tile_array_index = 5 + (tile_y * number_of_horizontal_tiles + tile_x) * (1 + faces_per_tile);

    uint num_faces_in_cur_tile = tiles[tile_array_index];

    // debug tiles
    // if (num_faces_in_cur_tile > 0) pixel_color = 0x00FF00FF;

    for (int face_i = 0; face_i < num_faces_in_cur_tile; ++face_i){
        __global RI_renderable_face *current_face = &renderable_faces[tiles[tile_array_index + face_i + 1]];
        
        if (!current_face->should_render) continue;
        
        RI_vector_2 uv_0 = current_face->uv_0;
        RI_vector_2 uv_1 = current_face->uv_1;
        RI_vector_2 uv_2 = current_face->uv_2;
        
        RI_vector_3 normal_0 = current_face->normal_0;
        RI_vector_3 normal_1 = current_face->normal_1;
        RI_vector_3 normal_2 = current_face->normal_2;
        
        RI_vector_3 pos_0 = current_face->position_0;
        RI_vector_3 pos_1 = current_face->position_1;
        RI_vector_3 pos_2 = current_face->position_2;
        
        if (x < current_face->min_screen_x || x > current_face->max_screen_x || y < current_face->min_screen_y || y > current_face->max_screen_y)
            continue;
        
        double denominator, w0, w1, w2;

        denominator = (pos_1.y - pos_2.y) * (pos_0.x - pos_2.x) + (pos_2.x - pos_1.x) * (pos_0.y - pos_2.y);
        
        if (denominator >= 0) continue; 
        
        w0 = ((pos_1.y - pos_2.y) * (x - pos_2.x) + (pos_2.x - pos_1.x) * (y - pos_2.y)) / denominator;
        w1 = ((pos_2.y - pos_0.y) * (x - pos_0.x) + (pos_0.x - pos_2.x) * (y - pos_0.y)) / denominator; 
        w2 = 1.0 - w0 - w1; 
        
        double w_over_z = (w0 / pos_0.z + w1 / pos_1.z + w2 / pos_2.z); 
        double interpolated_z = 1.0 / w_over_z;
        
        if (!(w0 >= 0 && w1 >= 0 && w2 >= 0)){    
            continue;
        }
        
        if (interpolated_z >= z){                        
            continue;
        }
        
        double alpha = 1;
        
        float ux = (w0 * (uv_0.x / pos_0.z) + w1 * (uv_1.x / pos_1.z) + w2 * (uv_2.x / pos_2.z)) / w_over_z;
        float uy = (w0 * (uv_0.y / pos_0.z) + w1 * (uv_1.y / pos_1.z) + w2 * (uv_2.y / pos_2.z)) / w_over_z;    

        RI_vector_3 interpolated_normal = {0};
        
        uint texel_x = current_face->texture_width * ux;
        uint texel_y = current_face->texture_height * uy;

        uint texel_index = current_face->texture_index + 
            texel_y * current_face->texture_width + texel_x;

        pixel_color = textures[texel_index];

        z = interpolated_z;
    }
    
    // debug tiles
    // if (pixel_x % tile_width == 0 || pixel_y % tile_height == 0) pixel_color = 0xFFFFFFFF;

    frame_buffer[idx] = pixel_color;

    return;
}
