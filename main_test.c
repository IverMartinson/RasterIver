#include "stdlib.h"
#include "SDL2/SDL.h"
#include "math.h"
#include "time.h"
#include <CL/cl.h>

const int WIDTH = 800;
const int HEIGHT = 800;
const int POLYGON_SIZE = sizeof(float) * 3 * 3;
const int POLYGONS = 5;

int is_intersecting(int a, int b, int c, int d, int p, int q, int r, int s) {
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
    float magnitude = sqrt((pow(a[0], 2) + pow(a[1], 2)));
    
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

const char* kernel_source = 
"__kernel void raster_kernel(__global float* polygons, __global uint* frame_buffer)\n"
"int id = get_global_id(0); \n"
" \n"
" \n"
"frame_buffer[id] = 80085; \n"
" \n"
"}\n";

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
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    cl_mem input_memory_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * 3 * 3 * POLYGONS, polygons, NULL);
    cl_mem output_memory_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * POLYGONS, NULL, NULL);

    cl_program kernel_program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, NULL);
    clBuildProgram(kernel_program, 1, &device, NULL, NULL, NULL);
    cl_kernel compiled_kernel = clCreateKernel(kernel_program, "raster_kernel", NULL);
    clSetKernelArg(compiled_kernel, 0, sizeof(cl_mem), &input_memory_buffer);
    clSetKernelArg(compiled_kernel, 1, sizeof(cl_mem), &output_memory_buffer);

    size_t size = POLYGONS;

    cl_uint pattern = 121212;
    // -----

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Rasterizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    int running = 1;

    int frame = 0;

    Uint32 start_time, frame_time;
    float fps;

    while (running) {
        start_time = SDL_GetTicks();

        //if (frame % 1 == 0){
            for (int p = 0; p < POLYGONS; p++){
                for (int point = 0; point < 3; point++){
                    for (int i = 0; i < 3; i++){
                        polygons[p][point][i] = rand() % WIDTH + 1;
                    }
                }
            }
        //}

        memset(&z_buffer, 0, sizeof(float) * WIDTH * HEIGHT);

        clEnqueueFillBuffer(queue, output_memory_buffer, &pattern, sizeof(cl_uint), 0, sizeof(cl_uint) * POLYGONS, 0, NULL, NULL);

        clEnqueueNDRangeKernel(queue, compiled_kernel, 1, NULL, &size, NULL, 0, NULL, NULL);

        clFinish(queue);

        clEnqueueReadBuffer(queue, output_memory_buffer, CL_TRUE, 0, sizeof(cl_uint) * POLYGONS, &frame_buffer, 0, NULL, NULL);

        for (int i = 0; i < POLYGONS; i++){
            printf("%u\n", frame_buffer[i]);
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                running = 0;
            }
        }
        
        // for (int polygon = 0; polygon < POLYGONS; polygon++){
        //     float x0 = polygons[polygon][0][0];
        //     float y0 = polygons[polygon][0][1];
        //     float z0 = polygons[polygon][0][2];
        //     float x1 = polygons[polygon][1][0];
        //     float y1 = polygons[polygon][1][1];
        //     float z1 = polygons[polygon][1][2];
        //     float x2 = polygons[polygon][2][0];
        //     float y2 = polygons[polygon][2][1];
        //     float z2 = polygons[polygon][2][2];
            
        //     float smallest_x = x0;
        //     float largest_x = x0;
        //     float smallest_y = y0;
        //     float largest_y = y0;
            
        //     for (int point = 0; point < 3; point++){
        //         float x = polygons[polygon][point][0];
        //         float y = polygons[polygon][point][1];
                
        //         if (x > largest_x){
        //             largest_x = x;
        //         }
                
        //         if (x < smallest_x){
        //             smallest_x = x;
        //         }
                
        //         if (y > largest_y){
        //             largest_y = y;
        //         }
                
        //         if (y < smallest_y){
        //             smallest_y = y;
        //         }
        //     }
            
        //     smallest_x = fmin(smallest_x, 0);
        //     largest_x = fmax(largest_x, WIDTH);
        //     smallest_y = fmin(smallest_y, 0);
        //     largest_y = fmax(largest_y, HEIGHT);

        //     // test every pixel in a rect around the triangle. If it's inside, color it.
        //     for (int x = (int)smallest_x; x < largest_x; x++){
        //         for (int y = (int)smallest_y; y < largest_y; y++){
        //             int intersections = 0;
                    
        //             for (int i = 0; i < 3; i++){
        //                 intersections += is_intersecting(x, y, 10000, 100000, polygons[polygon][i][0], polygons[polygon][i][1], polygons[polygon][(i + 1) % 3][0], polygons[polygon][(i + 1) % 3][1]);
        //             }

        //             if (intersections % 2 == 0){
        //                 continue;
        //             }

        //             float denominator = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
        //             float w0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / denominator;
        //             float w1 = ((y2 - y0) * (x - x0) + (x0 - x2) * (y - y2)) / denominator;
        //             float w2 = 1.0 - w0 - w1;

        //             if (denominator < 0) {
        //                 w0 = -w0;
        //                 w1 = -w1;
        //                 w2 = -w2;
        //                 denominator = -denominator;
        //             }                    

        //             float z = w0 * z0 + w1 * z1 + w2 * z2;

        //             if (z < z_buffer[y * WIDTH + x]){
        //                 z_buffer[y * WIDTH + x] = z;
        //             }
        //             else {
        //                 continue;
        //             }

        //             if (z < 0){
        //                 z *= -1;
        //             }

        //            frame_buffer[y * WIDTH + x] = 0xFFFFFFFF / POLYGONS * (polygon + 1);
        //         }
        //     }
        // }
        SDL_UpdateTexture(texture, NULL, frame_buffer, WIDTH * sizeof(cl_uint));
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        frame++;

        frame_time = SDL_GetTicks()-start_time;
        fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
        // printf("%f fps\n", fps);
        // printf("%d polygons\n", POLYGONS);
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
