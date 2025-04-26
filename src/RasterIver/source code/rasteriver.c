#include "stdlib.h"
#include "SDL2/SDL.h"
#include "math.h"
#include "time.h"
#include <CL/cl.h>
#include "../headers/rasteriver.h"
#include <stdarg.h>

const char* kernel_source = " \
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
__kernel void raster_kernel(__global float* polygons, __global uint* frame_buffer, int polygon_count, int width, int height, int show_z_buffer){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
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
            else { \
                continue; \
            } \
            \
            frame_pixel = 0xFFFFFFFF / polygon_count * (polygon + 1); \
        } \
    } \
    \
    frame_buffer[id_y * width + id_x] = frame_pixel; \
    \
    if (!show_z_buffer){return;}\
    \
    float z = clamp(z_pixel, 0.0f, 800.0f);\
    \
    float norm_z = z / 800.0f;\
    \
    uchar intensity = (uchar)(norm_z * 255.0f);\
    \
    frame_buffer[id_y * width + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
}\n";

typedef int RI_result;
typedef cl_uint RI_uint;

const RI_result RI_ERROR = -1;
const RI_result RI_SUCCESS = 0;
const RI_result RI_NOT_RUNNING = -2;
const RI_result RI_RUNNING = 1;

RI_result erchk_func(cl_int error, int line, char *file){
    if (error != CL_SUCCESS){
        printf("OpenCL Error: %d at line %d at file %s\n", error, line, file);
        return RI_ERROR;
    }
    
    return RI_SUCCESS;
}

#define erchk(error) erchk_func(error, __LINE__, __FILE__)

// ----- Rasteriver Vars
int width;
int height;

int show_z_buffer = 0;

int polygon_count = 20000;
float *polygons = NULL;

int running = 1;
int frame = 0;

int show_debug = 0;
// -----

// ----- Rendering Vars
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

RI_uint *frame_buffer;
float *z_buffer;
// -----

// ----- OpenCL Vars
cl_platform_id platform;
cl_device_id device;
RI_uint number_of_platforms, number_of_devices;

cl_int error;

cl_context context;
cl_command_queue queue;

cl_mem input_memory_buffer;
cl_mem output_memory_buffer;

cl_program kernel_program;
cl_kernel compiled_kernel;

size_t size_2d[2];

RI_uint pattern;
// -----

RI_result debug(char *string, ...){
    if (!show_debug){
        return RI_ERROR;
    }

    va_list args;
    va_start(args, string);

    char prefix[100] = "[RasterIver] ";
    
    strcat(prefix, string);

    vprintf(prefix, args);
    printf("\n");

    va_end(args);

    return RI_SUCCESS;
}

RI_result RI_SetDebugFlag(int RI_ShowDebugFlag){
    show_debug = RI_ShowDebugFlag;

    return RI_SUCCESS;
}

RI_result Rendering_init(char *title) {
    debug("Initializing Rendering...");

    frame_buffer = malloc(sizeof(RI_uint) * width * height);
    z_buffer = malloc(sizeof(float) * width * height);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        debug("SDL_Init failed");
        return RI_ERROR;
    }

    if (width <= 0 || height <= 0) {
        debug("Invalid width or height");
        return RI_ERROR;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (!window) {
        debug("SDL_CreateWindow failed");
        return RI_ERROR;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        debug("SDL_CreateRenderer failed");
        return RI_ERROR;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) {
        debug("SDL_CreateTexture failed");
        return RI_ERROR;
    }

    debug("Initialized Rendering");

    return RI_SUCCESS;
}

RI_result RI_ShowZBuffer(int RI_ShowZBufferFlag){
    show_z_buffer = RI_ShowZBufferFlag;

    return RI_SUCCESS;
}

RI_result RI_SetBackground(RI_uint RI_BackgroundColor){
    pattern = RI_BackgroundColor;
    
    return RI_SUCCESS;
}    

RI_result OpenCL_init(){
    debug("Initializing OpenCL...");

    clGetPlatformIDs(1, &platform, &number_of_platforms);
    
    if(number_of_platforms == 0){
        printf("No OpenCL Platforms\n");
        return RI_ERROR;
    }
    
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);
    
    if (number_of_devices == 0){
        printf("No Valid GPU's Found\n");
        return RI_ERROR;
    }
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    erchk(error);
    queue = clCreateCommandQueue(context, device, 0, &error);
    erchk(error);

    output_memory_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(RI_uint) * width * height, NULL, &error);
    erchk(error);

    kernel_program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &error);
    erchk(error);

    error = clBuildProgram(kernel_program, 1, &device, NULL, NULL, NULL);
    erchk(error);

    compiled_kernel = clCreateKernel(kernel_program, "raster_kernel", &error);
    erchk(error);

    erchk(clSetKernelArg(compiled_kernel, 0, sizeof(cl_mem), &input_memory_buffer));
    erchk(clSetKernelArg(compiled_kernel, 1, sizeof(cl_mem), &output_memory_buffer));
    erchk(clSetKernelArg(compiled_kernel, 2, sizeof(int), &polygon_count));
    erchk(clSetKernelArg(compiled_kernel, 3, sizeof(int), &width));
    erchk(clSetKernelArg(compiled_kernel, 4, sizeof(int), &height));
    erchk(clSetKernelArg(compiled_kernel, 5, sizeof(int), &show_z_buffer));
    
    size_2d[0] = width;
    size_2d[1] = height;
    
    pattern = 0x22222222;

    debug("Initialized OpenCL");
    
    return RI_SUCCESS;
}

RI_result RI_Stop(){
    debug("Stopping...");

    running = 0;

    clReleaseMemObject(input_memory_buffer);
    clReleaseMemObject(output_memory_buffer);
    clReleaseKernel(compiled_kernel);
    clReleaseProgram(kernel_program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (polygons != NULL)
        free(polygons);
    else
        debug("Polygons Was Unset on Stop");

    if (frame_buffer != NULL)
        free(frame_buffer);
    else
        debug("Frame-Buffer Was Unset on Stop");
    
    if (z_buffer != NULL)
        free(z_buffer);
    else   
        debug("Z-Buffer Was Unset on Stop");

    debug("Stopped");

    return RI_SUCCESS;
}

RI_result RI_RequestPolygons(int RI_PolygonsToRequest){
    polygon_count = RI_PolygonsToRequest;
    
    debug("Requesting %d Polygons...", polygon_count);

    if (polygons != NULL){
        free(polygons);
    }

    polygons = malloc(sizeof(float) * 3 * 3 * polygon_count);
    
    if (polygons == NULL){
        debug("Malloc Error");
        return RI_ERROR;
    }

    for (int p = 0; p < polygon_count; p++){
        for (int point = 0; point < 3; point++){
            for (int i = 0; i < 3; i++){
                polygons[i] = rand();
            }
        }
    }

    input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 3 * 3 * polygon_count, polygons, &error);
    
    if (input_memory_buffer == NULL) {
        debug("OpenCL buffer creation failed for polygons.");
    }

    debug("Request for %d Polygons Granted", polygon_count);
    
    return erchk(error);
}

RI_result RI_Tick(){
    debug("Ticking...");

    if (running) {
        if (polygons == NULL){
            debug("Polygons is Unset");
            return RI_ERROR;
        }

        if (frame_buffer == NULL){
            debug("Frame-Buffer is Unset");
            return RI_ERROR;
        }
        
        if (z_buffer == NULL){
            debug("Z-Buffer is Unset");
            return RI_ERROR;
        }

        if (frame % 1 == 0){
            for (int p = 0; p < polygon_count; p++){
                for (int point = 0; point < 3; point++){
                    for (int i = 0; i < 3; i++){
                        polygons[i] = rand() % width + 1;
                    }
                }
            }
        }

        //memset(&z_buffer, 0, sizeof(float) * width * height);

        erchk(clEnqueueWriteBuffer(queue, input_memory_buffer, CL_TRUE, 0, sizeof(float) * 3 * 3 * polygon_count, polygons, 0, NULL, NULL));

        erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * width * height, 0, NULL, NULL));

        erchk(clEnqueueNDRangeKernel(queue, compiled_kernel, 2, NULL, size_2d, NULL, 0, NULL, NULL));

        erchk(clFinish(queue));

        erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * width * height, &frame_buffer, 0, NULL, NULL));

        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                running = 0;
            }
        }
        
        SDL_UpdateTexture(texture, NULL, frame_buffer, width * sizeof(RI_uint));
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        frame++;
        
        debug("Ticked");

        return RI_SUCCESS;
    }
    else {
        return RI_ERROR;
    }
}

RI_result RI_IsRunning(){
    if (running){
        return RI_RUNNING;
    }
    else {
        return RI_NOT_RUNNING;
    }
}

RI_result RI_Init(int RI_WindowWidth, int RI_WindowHeight, char *RI_WindowTitle){
    srand(time(NULL));
    
    width = RI_WindowWidth;
    height = RI_WindowHeight;
        
    if(OpenCL_init() == RI_ERROR){
        return RI_ERROR;
    }

    Rendering_init(RI_WindowTitle);

    return RI_SUCCESS;
}
