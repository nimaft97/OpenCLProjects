#include <iostream>
#include "include/Data.h"
#include "include/common.h"
// OpenCL includes
#include <CL/cl.h>

int main()
{
    // read data
    int* host_data = data;

    // initialize OpenCL
    cl_int err = CL_SUCCESS;
    cl_platform_id platform_id;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_command_queue queue;

    clGetPlatformIDs(1, &platform_id, NULL); 

    // set the device
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err == CL_SUCCESS)
    {
        // at least one OpenCL capable GPU exists
        std::cout << "GPU found" << std::endl;
    }
    else
    {
        // default to CPU
        clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        std::cout << "No GPU found, switched back to CPU" << std::endl;
    }
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the context");
    queue = clCreateCommandQueue(context, device, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the queue");

    // create a program from kernel source code
    const auto file_name = "kernels.cl";
    const char* kernel_source  = readFile("kernels.cl").c_str();
    program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the program");
    // build the program
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // build kernel(s)
    cl_kernel kernel_prefix_sum = clCreateKernel(program, "prefixSum", &err);
    CHECK_CL_ERROR(err, "Couldn't create the prefixSum kernel");

    // code to be added

    // Release resources
    clReleaseKernel(kernel_prefix_sum);
    clReleaseProgram(program);
    clReleaseContext(context);
    
}