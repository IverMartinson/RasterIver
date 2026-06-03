#include "../headers/rasteriver.h"

#define MATSET(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, matrix) {\
    matrix[0][0] = a;\
    matrix[1][0] = b;\
    matrix[2][0] = c;\
    matrix[3][0] = d;\
    matrix[0][1] = e;\
    matrix[1][1] = f;\
    matrix[2][1] = g;\
    matrix[3][1] = h;\
    matrix[0][2] = i;\
    matrix[1][2] = j;\
    matrix[2][2] = k;\
    matrix[3][2] = l;\
    matrix[0][3] = m;\
    matrix[1][3] = n;\
    matrix[2][3] = o;\
    matrix[3][3] = p;\
} \
    
RI_context ri_context;

void RI_stop(){
    exit(0);
}

void throw(u8 is_fatal, char* message, char* extra){
    char message_buffer[512];
    
    sprintf(message_buffer, message, extra);
    
    if (is_fatal){ 
        printf("RI ran into a fatal problem.\n\nMessage: \"%s\"\n", message_buffer);
    
        RI_stop();
    } else 
        printf("RI ran into a recoverable problem.\n\nMessage: \"%s\"\n", message_buffer);

    return;
}

void RI_add_actor_to_scence(RI_scene* scene, RI_actor* actor){
    KT_append(&scene->actors, actor);
}

RI_actor* RI_new_actor(){
    RI_actor* actor = malloc(sizeof(RI_actor));

    actor->material = ri_context.default_material;
    actor->mesh = ri_context.default_mesh;
    actor->transform = (RI_transform){
        (MU_vec3d){0, 0, 0}, 
        (MU_vec3d){100, 100, 100}, 
        (MU_vec4d){1, 0, 0, 0},
        (RI_basis_vectors){
            (MU_vec3d){1, 0, 0},
            (MU_vec3d){0, 1, 0},
            (MU_vec3d){0, 0, 1}
        }
    };
    actor->matricies = (RI_actor_matricies){
        MU_new_matrix(4, 4),
        MU_new_matrix(4, 4),
        MU_new_matrix(4, 4),
        MU_new_matrix(4, 4)
    };

    return actor;
}

RI_scene* RI_new_scene(){
    RI_scene* scene = malloc(sizeof(RI_scene));

    scene->actors = KT_new_array(20);
    scene->cameras = KT_new_array(1);
    scene->frames_rendered = 0;
    scene->camera_rotation_matrix = MU_new_matrix(4, 4);
    scene->perspective_matrix = MU_new_matrix(4, 4);

    return scene;
}

RI_camera* RI_new_camera(){
    RI_camera* camera = malloc(sizeof(RI_camera));

    camera->FOV = MU_PI_2;
    camera->max_clip = 10000;
    camera->min_clip = 0.001;
    camera->transform.position = (RI_position){0, 0, 0};
    camera->transform.rotation = (RI_quaternion){1, 0, 0, 0};
    camera->transform.scale = (RI_scale){1, 1, 1};
    camera->transform.basis.x = (MU_vec3d){1, 0, 0};
    camera->transform.basis.y = (MU_vec3d){0, 1, 0};
    camera->transform.basis.z = (MU_vec3d){0, 0, 1};

    return camera;
}

RI_material* RI_new_material(){
    RI_material* material = malloc(sizeof(RI_material));

    material->normal_map = NULL;
    material->texture = ri_context.default_texture;

    return material;
}

RI_texture* RI_load_image(char* file_path, u16 frame_height, u16 frame_count){
    RI_texture* texture = PM_load_image(file_path, PM_ARGB, 0);

    if (!texture){
        throw(0, "image file not found \"%s\"", file_path);

        return ri_context.default_texture;
    }

    u8 is_custom_anim = frame_height;

    if (is_custom_anim){
        if (!frame_height) {
            throw(0, "frame height is zero \"%s\"", file_path);

            return ri_context.default_texture;
        }
        
        if (!frame_count){
            throw(0, "frame count is zero \"%s\"", file_path);

            return ri_context.default_texture;
        }

        texture->frame_height = frame_height;
    }

    return texture;
}

RI_mesh* RI_load_mesh(char* file_path){
    RI_mesh* mesh = malloc(sizeof(RI_mesh));

    FILE *file = fopen(file_path, "r");

    if (!file){
        throw(0, "mesh not found \"%s\"", file_path);
        
        if (ri_context.default_mesh == NULL){
            throw(1, "there is no default mesh set \"%s\"", file_path);
        }else
            return ri_context.default_mesh;
    }
    
    char line[512];

    // see how much stuff there is

    u32 triangle_count = 0;
    u32 vertex_count = 0;
    u32 normal_count = 0;
    u32 uv_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') { // triangle
            ++triangle_count;
        }
        else if (line[0] == 'v'){
            if (line[1] == ' ') { // vertex
                ++vertex_count;
            }
            else if (line[1] == 'n') { // normal
                ++normal_count;
            }
            else if (line[1] == 't') { // UV
                ++uv_count;
            }
        }
    }

    rewind(file);
    // allocate space for the stuff

    mesh->triangles = malloc(sizeof(RI_triangle) * triangle_count);
    mesh->original_verticies = malloc(sizeof(RI_vertex) * vertex_count);
    mesh->verticies = malloc(sizeof(RI_vertex) * vertex_count);
    mesh->original_normals = malloc(sizeof(RI_normal) * normal_count);
    mesh->normals = malloc(sizeof(RI_normal) * normal_count);
    mesh->uvs = malloc(sizeof(RI_uv) * uv_count);

    u32 current_triangle_index = 0;
    u32 current_vertex_index = 0;
    u32 current_normal_index = 0;
    u32 current_uv_index = 0;

    // read stuff
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'f' && line[1] == ' ') {
            u32 vertex_0_index, 
                vertex_1_index, 
                vertex_2_index, 
                normal_0_index, 
                normal_1_index, 
                normal_2_index, 
                uv_0_index, 
                uv_1_index, 
                uv_2_index
            ;

            // might have position normal & uv
            u8 matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d/", 
                &vertex_0_index, &uv_0_index, &normal_0_index, 
                &vertex_1_index, &uv_1_index, &normal_1_index, 
                &vertex_2_index, &uv_2_index, &normal_2_index);

            if (matches != 9){
                vertex_0_index = -1;
                vertex_1_index = -1;
                vertex_2_index = -1;
                
                normal_0_index = -1;
                normal_1_index = -1;
                normal_2_index = -1;
                
                uv_0_index = -1;
                uv_1_index = -1;
                uv_2_index = -1;

                // just has position and normals
                if (strchr(line, '/')){
                    sscanf(line, "f %d//%d %d//%d %d//%d", 
                        &vertex_0_index, &normal_0_index, 
                        &vertex_1_index, &normal_1_index, 
                        &vertex_2_index, &normal_2_index);
                }
                else { // only has position
                    sscanf(line, "f %d %d %d", 
                        &vertex_0_index, 
                        &vertex_1_index, 
                        &vertex_2_index);
                }
            }

            mesh->triangles[current_triangle_index].v0 = vertex_0_index - 1;
            mesh->triangles[current_triangle_index].v1 = vertex_1_index - 1;
            mesh->triangles[current_triangle_index].v2 = vertex_2_index - 1;

            mesh->triangles[current_triangle_index].n0 = normal_0_index - 1;
            mesh->triangles[current_triangle_index].n1 = normal_1_index - 1;
            mesh->triangles[current_triangle_index].n2 = normal_2_index - 1;
            
            mesh->triangles[current_triangle_index].u0 = uv_0_index - 1;
            mesh->triangles[current_triangle_index].u1 = uv_1_index - 1;
            mesh->triangles[current_triangle_index].u2 = uv_2_index - 1;

            ++current_triangle_index;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            
            sscanf(line, "v %lf %lf %lf", &x, &y, &z);

            mesh->original_verticies[current_vertex_index].x = x;
            mesh->original_verticies[current_vertex_index].y = y;
            mesh->original_verticies[current_vertex_index].z = z;

            ++current_vertex_index;
        } 
        else if (line[0] == 'v' && line[1] == 'n') {
            double x, y, z;
            
            sscanf(line, "vn %lf %lf %lf", &x, &y, &z);

            mesh->original_normals[current_normal_index].x = x;
            mesh->original_normals[current_normal_index].y = y;
            mesh->original_normals[current_normal_index].z = z;

            ++current_normal_index;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            double x, y, z;

            sscanf(line, "vt %lf %lf %lf", &x, &y, &z);

            mesh->uvs[current_uv_index].x = x;
            mesh->uvs[current_uv_index].y = y;
            // UVS are almost always 2D so we don't need Z (the type itself is a vector 2f, not 3f) 

            ++current_uv_index;
        } 
    }

    mesh->triangle_count = triangle_count;
    mesh->vertex_count = vertex_count;

    return mesh;
}

void RI_translation_matrix_from_transform(MU_matrix translation, RI_transform* transform){
    MATSET(
        1, 0, 0, transform->position.x,
        0, 1, 0, transform->position.y,
        0, 0, 1, transform->position.z,
        0, 0, 0, 1,
        translation
    );
}

void RI_rotation_matrix_from_transform(MU_matrix rotation, RI_transform* transform){
    double w = transform->rotation.w;
    double x = transform->rotation.x;
    double y = transform->rotation.y;
    double z = transform->rotation.z;

    MATSET(
        1.0 - 2.0*y*y - 2.0*z*z,       2.0*x*y - 2.0*z*w,       2.0*x*z + 2.0*y*w, 0,
        2.0*x*y + 2.0*z*w, 1.0 - 2.0*x*x - 2.0*z*z,       2.0*y*z - 2.0*x*w, 0,
        2.0*x*z - 2.0*y*w,       2.0*y*z + 2.0*x*w, 1.0 - 2.0*x*x - 2.0*y*y, 0,
        0,             0,             0,                1,
        rotation
    );
}

void RI_scalar_matrix_from_transform(MU_matrix scalar, RI_transform* transform){
    MATSET(
    transform->scale.x, 0, 0, 0,    
    0, transform->scale.y, 0, 0,
    0, 0, transform->scale.z ,0,
    0, 0, 0, 1,    
        scalar
    )
}

void RI_perspective_matrix_from_camera(MU_matrix perspective, RI_camera* camera, double aspect_ratio){
    double focal_length = 1.0 / (tan(camera->FOV/2.0));

    MATSET(
        focal_length / aspect_ratio, 0, 0, 0,
        0, focal_length, 0, 0,
        0, 0, -((camera->max_clip + camera->min_clip) / (camera->max_clip - camera->min_clip)), -((2.0 * camera->max_clip * camera->min_clip) / (camera->max_clip - camera->min_clip)),
        0, 0, -1, 0,
        perspective
    );
}

void write_pixel(RI_window* window, u16 x, u16 y, u32 color){
    *(u32*)((u8*)window->sdl_surface->pixels + y * window->sdl_pitch + x * sizeof(u32)) = color;
}

double get_area_of_triangle(MU_vec2d a, MU_vec2d b, MU_vec2d c){
    MU_vec2d bc = MU_2d_perpendicular(MU_2d_sub_2d(b,c));
    MU_vec2d ab = MU_2d_sub_2d(a,b);

    double result = MU_2d_dot(bc, ab) / 2.0;
    
    return result;
}

#include <stdio.h>

void RI_render(RI_scene* scene, u8 camera_index, RI_window* window){
    if (SDL_MUSTLOCK(window->sdl_surface)) {
        SDL_LockSurface(window->sdl_surface);
    }

    RI_camera* camera = scene->cameras[camera_index];
    
    RI_rotation_matrix_from_transform(scene->camera_rotation_matrix, &camera->transform);
    scene->camera_rotation_matrix = MU_matrix_transpose(scene->camera_rotation_matrix);

    RI_perspective_matrix_from_camera(scene->perspective_matrix, camera, (double)window->width / (double)window->height);

    for (u32 ath = 0; ath < KT_get_size(&scene->actors); ath++){
        RI_actor* actor = scene->actors[ath];
        RI_transform transform = actor->transform;

        MU_3d_sub_3d_pointer(&transform.position, &camera->transform.position);

        RI_translation_matrix_from_transform(actor->matricies.translation_matrix, &transform);
        
        RI_rotation_matrix_from_transform(actor->matricies.rotation_matrix, &transform);

        RI_scalar_matrix_from_transform(actor->matricies.scaling_matrix, &transform);
        
        MU_4x4_x_4x4_to_c(actor->matricies.scaling_matrix,  actor->matricies.rotation_matrix,    ri_context.intermidiate_matrix_a);
        MU_4x4_x_4x4_to_c(ri_context.intermidiate_matrix_a, actor->matricies.translation_matrix, ri_context.intermidiate_matrix_b);
        MU_4x4_x_4x4_to_c(ri_context.intermidiate_matrix_b, scene->camera_rotation_matrix,       ri_context.intermidiate_matrix_a);
        MU_4x4_x_4x4_to_c(ri_context.intermidiate_matrix_a, scene->perspective_matrix,           actor->matricies.final_matrix);

        for (u32 vth = 0; vth < actor->mesh->vertex_count; vth++){
            MU_vec3d in = actor->mesh->original_verticies[vth];
            MU_vec3d out;
            
            out.x = in.x * actor->matricies.final_matrix[0][0] + in.y * actor->matricies.final_matrix[0][1] + in.z * actor->matricies.final_matrix[0][2] + actor->matricies.final_matrix[0][3];
            out.y = in.x * actor->matricies.final_matrix[1][0] + in.y * actor->matricies.final_matrix[1][1] + in.z * actor->matricies.final_matrix[1][2] + actor->matricies.final_matrix[1][3];
            out.z = in.x * actor->matricies.final_matrix[2][0] + in.y * actor->matricies.final_matrix[2][1] + in.z * actor->matricies.final_matrix[2][2] + actor->matricies.final_matrix[2][3];
            float w = in.x * actor->matricies.final_matrix[3][0] + in.y * actor->matricies.final_matrix[3][1] + in.z * actor->matricies.final_matrix[3][2] + actor->matricies.final_matrix[3][3];

            out.x = out.x / w * window->half_width + window->half_width; 
            out.y = out.y / w * window->half_height + window->half_height; 
            out.z = w;

            actor->mesh->verticies[vth] = out;
        }


        for (u32 tth = 0; tth < actor->mesh->triangle_count; tth++){
            RI_triangle* triangle = &actor->mesh->triangles[tth];
            
            MU_vec3d v0 = actor->mesh->verticies[triangle->v0];
            MU_vec3d v1 = actor->mesh->verticies[triangle->v1];
            MU_vec3d v2 = actor->mesh->verticies[triangle->v2];
            
            MU_vec2d u0 = actor->mesh->uvs[triangle->u0];
            MU_vec2d u1 = actor->mesh->uvs[triangle->u1];
            MU_vec2d u2 = actor->mesh->uvs[triangle->u2];

            //transforming into screenspace
            MU_vec2d screen_point_a = {v0.x, v0.y};
            MU_vec2d screen_point_b = {v1.x, v1.y};
            MU_vec2d screen_point_c = {v2.x, v2.y};

            MU_vec3d depths = {v0.z, v1.z, v2.z};

            MU_vec3d weights;
            float triangle_area = get_area_of_triangle(screen_point_a, screen_point_b, screen_point_c);

            MU_vec2d bottom_left = {fmax(fmin(screen_point_a.x, fmin(screen_point_b.x, screen_point_c.x)), 0),
                                    fmax(fmin(screen_point_a.y, fmin(screen_point_b.y, screen_point_c.y)), 0)};
            MU_vec2d top_right = {fmin(fmax(screen_point_a.x, fmax(screen_point_b.x, screen_point_c.x)), window->width - 1),
                                  fmin(fmax(screen_point_a.y, fmax(screen_point_b.y, screen_point_c.y)), window->height - 1)};
       
            if(triangle_area < 0)
                continue;

            for(int y = bottom_left.y; y <= top_right.y; y++){
                for(int x = bottom_left.x; x <= top_right.x; x++){
                    MU_vec2d pixel = {x, y};
                    
                    weights = (MU_vec3d){
                        get_area_of_triangle(pixel, screen_point_b, screen_point_c) / triangle_area,
                        get_area_of_triangle(pixel, screen_point_c, screen_point_a) / triangle_area,
                        get_area_of_triangle(pixel, screen_point_a, screen_point_b) / triangle_area
                    };
                    
                    if (!(weights.x >= 0 && weights.y >= 0 && weights.z >= 0)){    
                        continue;
                    }
                    
                    double z = (weights.x / depths.x + weights.y / depths.y + weights.z / depths.z); 

                    double* z_buffer = &window->z_buffer[y * window->width + x];

                    if (z >= *z_buffer)
                        continue;

                    *z_buffer = z;

                    float ux = (weights.x * (u0.x / depths.x) + weights.y * (u1.x / depths.y) + weights.z * (u2.x / depths.z)) / z;
                    float uy = (weights.x * (u0.y / depths.x) + weights.y * (u1.y / depths.y) + weights.z * (u2.y / depths.z)) / z;    

                    if (ux < 0) ux += 1.0;
                    if (uy < 0) uy += 1.0;

                    RI_texture* texture = actor->material->texture;

                    u32 texel_x = texture->width * (1.0 - ux);
                    u32 texel_y = texture->frame_height * uy + texture->frame_height * (actor->material->current_frame % texture->frame_count);

                    u32 texel_index = (texel_y * texture->width + texel_x) % (texture->width * texture->height - 1);
                    
                    double value = (z / 2.0);

                    write_pixel(window, x, y, texture->frame_buffer[texel_index]);
                }
            }            
        }
    }

    if (SDL_MUSTLOCK(window->sdl_surface)) {
        SDL_UnlockSurface(window->sdl_surface);
    }

    scene->frames_rendered++;
}

void RI_present(RI_window* window){
    SDL_UpdateWindowSurface(window->sdl_window);

    memset((u8*)window->sdl_surface->pixels, 0xFF, window->height * window->sdl_pitch);
    memset(window->z_buffer, 0xFF, window->height * window->width * sizeof(double));
}

RI_window* RI_init_window(char* title, u16 width, u16 height){
    RI_window* window = malloc(sizeof(RI_window));
    
    window->sdl_window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width, 
        height, 0
    );
    
    window->sdl_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_TARGETTEXTURE);

    window->sdl_surface = SDL_GetWindowSurface(window->sdl_window);

    window->width = width;
    window->height = height;
    window->half_width = width / 2;
    window->half_height = height / 2;

    window->z_buffer = malloc(sizeof(double) * width * height);

    window->sdl_pitch = window->sdl_surface->pitch;

    return window;
}

void RI_init(){
    SDL_Init(SDL_INIT_VIDEO);

    ri_context.default_texture = RI_load_image("textures/missing_texture.bmp", 0, 0);
    ri_context.default_material = RI_new_material();
    ri_context.default_mesh = RI_load_mesh("objects/error.obj");

    ri_context.identity_matrix = MU_new_matrix(4, 4);
    ri_context.intermidiate_matrix_a = MU_new_matrix(4, 4);
    ri_context.intermidiate_matrix_b = MU_new_matrix(4, 4);

    MATSET(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
        ri_context.identity_matrix
    );
}