#include "stdlib.h"
#include "SDL2/SDL.h"
#include "math.h"
#include "time.h"
#include <CL/cl.h>
#include "../headers/rasteriver.h"
#include <stdarg.h>

const char *kernel_source = " \
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

// ----- Internal Variables
int width;
int height;

int show_z_buffer = 0;
float highest_z = 0;

int polygon_count;
RI_polygons polygons = NULL;

int running = 1;
int frame = 0;

int show_debug = 0;
int debug_verbose = 0;
int show_fps = 0;
int debug_fps = 0;

Uint64 start_time;
double frame_time_ms;
double fps;
double elapsed_ticks;
double delta_time;
int fps_cap = -1;
// ----- Internal Variables

// ----- Rendering Vars
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

RI_uint *frame_buffer;
float *z_buffer;
// ----- Rendering Vars

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
size_t local_size;

RI_uint pattern;
// ----- OpenCL Vars

// ----- Internal Functions
RI_result debug(int verbose, char *string, ...)
{
    if (!show_debug || (verbose && !debug_verbose))
    {
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

RI_result erchk_func(cl_int error, int line, char *file)
{
    if (error != CL_SUCCESS)
    {
        debug(1, "OpenCL Error: %d at line %d at file %s", error, line, file);

        return RI_ERROR;
    }

    return RI_SUCCESS;
}

#define erchk(error) erchk_func(error, __LINE__, __FILE__)
// ----- Internal Functions

// ----- Value Return Functions
RI_result RI_IsRunning()
{
    if (running)
    {
        return RI_RUNNING;
    }
    else
    {
        return RI_NOT_RUNNING;
    }
}

RI_result RI_ListFlags(){
    printf("RI_FLAG_DEBUG: Turns debugging on or off\n");
    printf("RI_FLAG_DEBUG_VERBOSE: If debugging and verbose is on, print extra data\n");
    printf("RI_FLAG_SHOW_Z_BUFFER: Whether or not to render the Z buffer");
    printf("RI_FLAG_SHOW_FPS: Render FPS on screen");
    printf("RI_FLAG_DEBUG_FPS: Debug FPS into the console");

    return RI_SUCCESS;
}
// ----- Value Return Functions

// ----- Set Value Functions
RI_result RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value)
{
    switch (RI_FlagToSet)
    {
    case RI_FLAG_DEBUG:
        show_debug = RI_Value;
        break;

    case RI_FLAG_DEBUG_VERBOSE:
        debug_verbose = RI_Value;
        break;

    case RI_FLAG_SHOW_Z_BUFFER:
        show_z_buffer = RI_Value;
        break;

    case RI_FLAG_SHOW_FPS:
        show_fps = RI_Value;
        break;

    case RI_FLAG_DEBUG_FPS:
        debug_fps = RI_Value;
        break;

    default:
        return RI_INVALID_FLAG;
    }

    return RI_SUCCESS;
}

RI_result RI_SetBackground(RI_uint RI_BackgroundColor)
{
    pattern = RI_BackgroundColor;

    return RI_SUCCESS;
}

RI_polygons RI_RequestPolygons(int RI_PolygonsToRequest){
    polygon_count = RI_PolygonsToRequest;
    
    int size = sizeof(float) * 3 * 3 * polygon_count;
    
    debug(1, "Requesting %d Polygons... (%d bytes)", polygon_count, size);
    
    if (polygons != NULL)
    {
        free(polygons);
    }
    
    polygons = malloc(size);
    
    if (polygons == NULL)
    {
        debug(1, "Malloc Error");
        return (float*)RI_ERROR;
    }
    
    for (int p = 0; p < polygon_count * 9; p++)
    {
        polygons[p] = rand() % 800;
    }
    
    input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, polygons, &error);
    
    if (input_memory_buffer == NULL)
    {
        debug(1, "OpenCL buffer creation failed for polygons.");
    }
    
    debug(1, "Request for %d Polygons Granted", polygon_count);
    
    return polygons;
}

RI_result RI_SetFpsCap(int RI_FpsCap){
    fps_cap = RI_FpsCap;

    return RI_SUCCESS;
}


// ----- Renderer Action Functions
RI_result RI_Tick()
{
    if (show_fps || debug_fps){
        start_time = SDL_GetPerformanceCounter();
    }
    
    debug(1, "Ticking...");
    
    if (running)
    {
        if (polygons == NULL)
        {
            debug(1, "Polygons is not Allocated");
            return RI_ERROR;
        }

        if (frame_buffer == NULL)
        {
            debug(1, "Frame Buffer is not Allocated");
            return RI_ERROR;
        }
        
        if (show_z_buffer){
            for (int p = 2; p < polygon_count * 9; p+=3){
                if (polygons[p] > highest_z){
                    highest_z = polygons[p];
                }
            }
            
            debug(1, "Highest Z: %f", highest_z);
        }

        erchk(clSetKernelArg(compiled_kernel, 0, sizeof(cl_mem), &input_memory_buffer));
        erchk(clSetKernelArg(compiled_kernel, 1, sizeof(cl_mem), &output_memory_buffer));
        erchk(clSetKernelArg(compiled_kernel, 2, sizeof(int), (void*)&polygon_count));
        erchk(clSetKernelArg(compiled_kernel, 3, sizeof(int), (void*)&width));
        erchk(clSetKernelArg(compiled_kernel, 4, sizeof(int), (void*)&height));
        erchk(clSetKernelArg(compiled_kernel, 5, sizeof(int), (void*)&show_z_buffer)); 
        erchk(clSetKernelArg(compiled_kernel, 6, sizeof(float), (void*)&highest_z));

        erchk(clEnqueueWriteBuffer(queue, input_memory_buffer, CL_TRUE, 0, sizeof(float) * 3 * 3 * polygon_count, polygons, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Wrote Polygon Buffer");

        erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(RI_uint), 0, sizeof(RI_uint) * width * height, 0, NULL, NULL));
        erchk(clFinish(queue));

        debug(1, "Cleared Frame Buffer");

        size_t local_size_2d[2] = {sqrt(local_size), sqrt(local_size)};

        // for (int i = 0; i < passes; i++)
        // {
        erchk(clEnqueueNDRangeKernel(queue, compiled_kernel, 2, NULL, size_2d, local_size_2d, 0, NULL, NULL));
        erchk(clFinish(queue));

            // debug(1, "Ran Kernel (pass %d/%d)", i + 1, passes);
        // }

        erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(RI_uint) * width * height, frame_buffer, 0, NULL, NULL));
        erchk(clFinish(queue));
        debug(1, "Read Frame Buffer");

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
            }
        }

        SDL_UpdateTexture(texture, NULL, frame_buffer, width * sizeof(RI_uint));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame++;

        debug(1, "Ticked");

        if (fps_cap > 0 && fps > fps_cap){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            
            double target_frame_time = 1.0 / fps_cap;

            SDL_Delay((Uint32)((target_frame_time - delta_time) * 1000.0));
        }
        
        if (show_fps || debug_fps){
            elapsed_ticks = SDL_GetPerformanceCounter() - start_time;
            delta_time = elapsed_ticks / (double)SDL_GetPerformanceFrequency();
            fps = 1.0 / delta_time;
        }
        
        if (debug_fps){
            debug(0, "FPS: %lf (%d polygons, %d pixels)", fps, polygon_count, width * height);
        }

        return RI_SUCCESS;
    }
    else
    {
        return RI_ERROR;
    }
}

RI_result RI_Stop()
{
    debug(0, "Stopping...");

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
        debug(1, "Polygons Was Unset on Stop");

    if (frame_buffer != NULL)
        free(frame_buffer);
    else
        debug(1, "Frame-Buffer Was Unset on Stop");

    debug(0, "Stopped");

    return RI_SUCCESS;
}
// ----- Renderer Action Functions

// ----- INIT
RI_result Rendering_init(char *title)
{
    frame_buffer = malloc(sizeof(RI_uint) * width * height);

    if (frame_buffer == NULL)
    {
        debug(1, "Couldn't Allocate Frame Buffer");
        return RI_ERROR;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        debug(1, "SDL_Init failed");
        return RI_ERROR;
    }

    if (width <= 0 || height <= 0)
    {
        debug(1, "Invalid width or height");
        return RI_ERROR;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (!window)
    {
        debug(1, "SDL_CreateWindow failed");
        return RI_ERROR;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        debug(1, "SDL_CreateRenderer failed");
        return RI_ERROR;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture)
    {
        debug(1, "SDL_CreateTexture failed");
        return RI_ERROR;
    }

    debug(0, "Initialized Rendering");

    return RI_SUCCESS;
}

RI_result OpenCL_init(){
    clGetPlatformIDs(1, &platform, &number_of_platforms);

    if (number_of_platforms == 0)
    {
        debug(1, "No OpenCL Platforms");
        return RI_ERROR;
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);

    if (number_of_devices == 0)
    {
        debug(1, "No Valid GPU's Found");
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

    erchk(clGetKernelWorkGroupInfo(compiled_kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL));

    debug(1, "Local Size: %d", local_size);

    size_2d[0] = width;
    size_2d[1] = height;

    pattern = 0x22222222;

    debug(0, "Initialized OpenCL");

    return RI_SUCCESS;
}

RI_result RI_Init(int RI_WindowWidth, int RI_WindowHeight, char *RI_WindowTitle)
{
    srand(time(NULL));

    width = RI_WindowWidth;
    height = RI_WindowHeight;

    if (OpenCL_init() == RI_ERROR)
    {
        return RI_ERROR;
    }

    Rendering_init(RI_WindowTitle);

    return RI_SUCCESS;
}
// ----- INIT