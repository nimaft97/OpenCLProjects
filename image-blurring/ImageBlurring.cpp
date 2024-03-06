#include "include/Data.h"
#include "include/common.h"
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
// OpenCL includes
#include <CL/cl.h>

int main()
{
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
    const auto file_name = "kernels.clh";
    const auto kernel_source_string = readFile(file_name);
    const char* kernel_source = kernel_source_string.c_str();
    program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the program");
    // build the program
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    // read data
    std::vector<int> host_data = data;  // copy
    const size_t length = host_data.size();
    const size_t size_in_byte = length * sizeof(decltype(host_data.at(0)));
    // create buffer(s)
    cl_mem device_data = clCreateBuffer(context, CL_MEM_READ_WRITE, size_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");
    
    // transfer data to GPU
    clEnqueueWriteBuffer(queue, device_data, CL_TRUE, 0, size_in_byte, host_data.data(), 0, NULL, NULL);

    // build kernel(s)
    cl_kernel kernel_image_blurring = clCreateKernel(program, "imageBlurring", &err);
    CHECK_CL_ERROR(err, "Couldn't create the image blurring kernel");

    // set kernel args
    err = clSetKernelArg(kernel_image_blurring, 0, sizeof(cl_mem), &device_data);
    CHECK_CL_ERROR(err, "Couldn't set arg 1");
    clSetKernelArg(kernel_image_blurring, 1, sizeof(length), &length);
    CHECK_CL_ERROR(err, "Couldn't set arg 2");

    // set global and local sizes (grid and block sizes)
    size_t global_size = 32u;
    size_t local_size = 32u;

    // enqueue the kernel for execution
    err = clEnqueueNDRangeKernel(queue, kernel_image_blurring, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "Couldn't launch the prefixSum kernel");

    // wait until execution of the kernel is over
    err = clFinish(queue);
    CHECK_CL_ERROR(err, "Couldn't empty the queue");

    // read the kernel's output
    clEnqueueReadBuffer(queue, device_data, CL_TRUE, 0, size_in_byte, host_data.data(), 0, NULL, NULL);

    // wait until data is compeletely read from device 
    err = clFinish(queue);
    CHECK_CL_ERROR(err, "Couldn't empty the queue");

    // Release resources
    clReleaseMemObject(device_data);
    clReleaseKernel(kernel_image_blurring);
    clReleaseProgram(program);
    clReleaseContext(context);
    
    // write the result to disk
    const std::string output_file_name = "out.txt";
    std::ofstream out(output_file_name);
    if (out.is_open())
    {
        // copy the content of host_data to disk
        std::copy(host_data.cbegin(), host_data.cend(), std::ostream_iterator<int>(out, " "));
        out.close();
    }
    else
    {
        std::cerr << "Couldn't open the output file" << std::endl;
    }

    return 0;
}