#include "stdlib.h"
#include "SDL2/SDL.h"
#include "math.h"
#include "time.h"
#include <CL/cl.h>

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
__kernel void raster_kernel(__global float* polygons, __global uint* frame_buffer, int POLYGONS, int WIDTH, int HEIGHT, int SHOW_Z_BUFFER){ \
    int id_x = get_global_id(0); \
    int id_y = get_global_id(1); \
    \
    float z_pixel = 0; \
    uint frame_pixel = 0x22222222; \
    \
    for (int polygon = 0; polygon < POLYGONS; polygon++){ \
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
        largest_x = fmax(largest_x, WIDTH); \
        smallest_y = fmin(smallest_y, 0); \
        largest_y = fmax(largest_y, HEIGHT); \
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
            frame_pixel = 0xFFFFFFFF / POLYGONS * (polygon + 1); \
        } \
    } \
    \
    frame_buffer[id_y * WIDTH + id_x] = frame_pixel; \
    \
    if (!SHOW_Z_BUFFER){return;}\
    \
    float z = clamp(z_pixel, 0.0f, 800.0f);\
    \
    float norm_z = z / 800.0f;\
    \
    uchar intensity = (uchar)(norm_z * 255.0f);\
    \
    frame_buffer[id_y * WIDTH + id_x] = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;\
}\n";

void erchk_func(cl_int error, int line, char *file){
    if (error != CL_SUCCESS){
        printf("ERROR :O   %d, line %d at file %s\n", error, line, file);
        exit(1);
    }
}

#define erchk(error) erchk_func(error, __LINE__, __FILE__)

const int WIDTH = 800;
const int HEIGHT = 800;
const int POLYGONS = 20000;
const int SHOW_Z_BUFFER = 0;

int main(){
    srand(time(NULL));

    float polygons[POLYGONS][3][3];

    cl_uint frame_buffer[WIDTH * HEIGHT];
    float z_buffer[WIDTH * HEIGHT];

    // ----- Check for Valid Platforms & GPUs
    cl_platform_id platform;
    cl_device_id device;
    cl_uint number_of_platforms, number_of_devices;

    clGetPlatformIDs(1, &platform, &number_of_platforms);

    if(number_of_platforms == 0){
        printf("No OpenCL Platforms");
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &number_of_devices);

    if (number_of_devices == 0){
        printf("No GPU's Found");
    }
    // -----

    // ----- Setup OpenCL
    cl_int error;

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    cl_mem input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 3 * 3 * POLYGONS, polygons, &error);
    erchk(error);

    cl_mem output_memory_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * WIDTH * HEIGHT, NULL, &error);
    erchk(error);

    printf("%ld bytes\n", sizeof(float) * 3 * 3 * POLYGONS + WIDTH * HEIGHT * sizeof(cl_uint));

    cl_program kernel_program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &error);
    erchk(error);

    error = clBuildProgram(kernel_program, 1, &device, NULL, NULL, NULL);
    erchk(error);

    cl_kernel compiled_kernel = clCreateKernel(kernel_program, "raster_kernel", &error);
    erchk(error);

    erchk(clSetKernelArg(compiled_kernel, 0, sizeof(cl_mem), &input_memory_buffer));
    erchk(clSetKernelArg(compiled_kernel, 1, sizeof(cl_mem), &output_memory_buffer));
    erchk(clSetKernelArg(compiled_kernel, 2, sizeof(int), &POLYGONS));
    erchk(clSetKernelArg(compiled_kernel, 3, sizeof(int), &WIDTH));
    erchk(clSetKernelArg(compiled_kernel, 4, sizeof(int), &HEIGHT));
    erchk(clSetKernelArg(compiled_kernel, 5, sizeof(int), &SHOW_Z_BUFFER));

    size_t size_2d[2] = {WIDTH, HEIGHT};

    cl_uint pattern = 0x22222222;
    // -----

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Rasterizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    int running = 1;

    int frame = 0;

    Uint64 start_time;
    double frame_time_ms;
    double fps;

    while (running) {
        start_time = SDL_GetPerformanceCounter();

        if (frame % 1 == 0){
            for (int p = 0; p < POLYGONS; p++){
                for (int point = 0; point < 3; point++){
                    for (int i = 0; i < 3; i++){
                        polygons[p][point][i] = rand() % WIDTH + 1;
                    }
                }
            }
        }

        memset(&z_buffer, 0, sizeof(float) * WIDTH * HEIGHT);

        erchk(clEnqueueWriteBuffer(queue, input_memory_buffer, CL_TRUE, 0, sizeof(float) * 3 * 3 * POLYGONS, polygons, 0, NULL, NULL));

        erchk(clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(cl_uint), 0, sizeof(cl_uint) * WIDTH * HEIGHT, 0, NULL, NULL));

        erchk(clEnqueueNDRangeKernel(queue, compiled_kernel, 2, NULL, size_2d, NULL, 0, NULL, NULL));

        erchk(clFinish(queue));

        erchk(clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(cl_uint) * WIDTH * HEIGHT, &frame_buffer, 0, NULL, NULL));

        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                running = 0;
            }
        }
        
        SDL_UpdateTexture(texture, NULL, frame_buffer, WIDTH * sizeof(cl_uint));
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        frame++;

        double delta_time = (double)(SDL_GetPerformanceCounter() - start_time) / (double)SDL_GetPerformanceFrequency();
        double fps = 1.0 / delta_time;

        printf("%lf fps\n", fps);
        printf("%d polygons\n", POLYGONS);
    }
    
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
}
