#include <CL/cl.h>
#include <stdio.h>

#define DATA_SIZE 10

const char *kernelSource = 
"__kernel void vector_add(__global int *A, __global int *B, __global int *C) {\n"
"    int id = get_global_id(0);\n"
"    C[id] = A[id] + B[id];\n"
"}\n";

int main() {
    // Data
    int A[DATA_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int B[DATA_SIZE] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int C[DATA_SIZE];

    // Load the kernel source code
    cl_program program;
    cl_kernel kernel;
    cl_mem bufferA, bufferB, bufferC;
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, NULL);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "vector_add", NULL);

    bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * DATA_SIZE, NULL, NULL);
    bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * DATA_SIZE, NULL, NULL);
    bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * DATA_SIZE, NULL, NULL);

    clEnqueueWriteBuffer(queue, bufferA, CL_TRUE, 0, sizeof(int) * DATA_SIZE, A, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufferB, CL_TRUE, 0, sizeof(int) * DATA_SIZE, B, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);

    size_t globalSize = DATA_SIZE;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    clFinish(queue);

    clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, sizeof(int) * DATA_SIZE, C, 0, NULL, NULL);

    // Print the result
    for (int i = 0; i < DATA_SIZE; i++) {
        printf("%d + %d = %d\n", A[i], B[i], C[i]);
    }

    // Clean up
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
