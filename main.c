#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "❌ %s failed (%d)\n", msg, err); \
        exit(EXIT_FAILURE); \
    }

void print_platform_info(cl_platform_id platform) {
    char buffer[1024];
    printf("\n🌍 Platform Info:\n");

    clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL);
    printf("🏷️ Name: %s\n", buffer);

    clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL);
    printf("🏢 Vendor: %s\n", buffer);

    clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(buffer), buffer, NULL);
    printf("💿 Version: %s\n", buffer);

    clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, sizeof(buffer), buffer, NULL);
    printf("🧩 Profile: %s\n", buffer);

    printf("\n");
}

void print_device_info(cl_device_id device) {
    char name[256], vendor[256], version[256];
    cl_uint compute_units;
    cl_ulong global_mem;
    cl_ulong local_mem;
    size_t max_wg;

    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(version), version, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem), &global_mem, NULL);
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem), &local_mem, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_wg), &max_wg, NULL);

    printf("🔹 Device: %s\n", name);
    printf("   Vendor: %s\n", vendor);
    printf("   Version: %s\n", version);
    printf("   Compute Units: %u\n", compute_units);
    printf("   Global Memory: %.2f MB\n", global_mem / (1024.0 * 1024.0));
    printf("   Local Memory: %.2f KB\n", local_mem / 1024.0);
    printf("   Max Work Group Size: %zu\n\n", max_wg);
}

int main(void) {
    cl_int err;

    printf("🚀 Starting OpenCL diagnostic + compute test\n");

    // 1. Get all platforms
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    CHECK_ERROR(err, "clGetPlatformIDs(count)");

    cl_platform_id *platforms = malloc(sizeof(cl_platform_id) * num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    CHECK_ERROR(err, "clGetPlatformIDs(list)");

    printf("🌍 Found %u OpenCL platform(s)\n", num_platforms);

    // 2. List platforms and pick POCL if available
    cl_platform_id chosen_platform = NULL;
    char pname[256];
    for (cl_uint i = 0; i < num_platforms; i++) {
        print_platform_info(platforms[i]);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(pname), pname, NULL);

        if (strstr(pname, "Portable") || strstr(pname, "pocl") || strstr(pname, "POCL")) {
            chosen_platform = platforms[i];
        }
    }

    if (!chosen_platform) {
        printf("⚠️  No POCL platform found, using first available.\n");
        chosen_platform = platforms[0];
    }

    clGetPlatformInfo(chosen_platform, CL_PLATFORM_NAME, sizeof(pname), pname, NULL);
    printf("✅ Selected platform: %s\n", pname);

    // 3. Get devices
    cl_uint num_devices = 0;
    err = clGetDeviceIDs(chosen_platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    CHECK_ERROR(err, "clGetDeviceIDs(count)");

    cl_device_id *devices = malloc(sizeof(cl_device_id) * num_devices);
    err = clGetDeviceIDs(chosen_platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
    CHECK_ERROR(err, "clGetDeviceIDs(list)");

    printf("🧩 Found %u device(s)\n\n", num_devices);

    for (cl_uint i = 0; i < num_devices; i++) {
        print_device_info(devices[i]);
    }

    cl_device_id device = devices[0];

    // 4. Create context and queue
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err, "clCreateContext");

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK_ERROR(err, "clCreateCommandQueue");

    // 5. Example kernel
    const char *source =
        "__kernel void vector_add(__global const float* a, __global const float* b, __global float* c) {"
        "    int id = get_global_id(0);"
        "    c[id] = a[id] + b[id];"
        "}";

    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
    CHECK_ERROR(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        fprintf(stderr, "❌ Build log:\n%s\n", log);
        free(log);
        CHECK_ERROR(err, "clBuildProgram");
    }

    printf("✅ Program built successfully.\n");

    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    CHECK_ERROR(err, "clCreateKernel");

    // 6. Prepare data
    const int N = 10;
    float A[N], B[N], C[N];
    for (int i = 0; i < N; i++) {
        A[i] = (float)i;
        B[i] = (float)(N - i);
    }

    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(A), A, &err);
    CHECK_ERROR(err, "clCreateBuffer(A)");

    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(B), B, &err);
    CHECK_ERROR(err, "clCreateBuffer(B)");

    cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(C), NULL, &err);
    CHECK_ERROR(err, "clCreateBuffer(C)");

    // 7. Set kernel args
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
    CHECK_ERROR(err, "clSetKernelArg");

    // 8. Run kernel
    size_t global = N;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueNDRangeKernel");

    clFinish(queue);

    // 9. Read results
    err = clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(C), C, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueReadBuffer");

    printf("\n✅ Kernel executed successfully. Results:\n");
    for (int i = 0; i < N; i++) {
        printf("  %.2f + %.2f = %.2f\n", A[i], B[i], C[i]);
    }

    // 10. Cleanup
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(platforms);
    free(devices);

    printf("\n🎉 All done!\n");
    return 0;
}
