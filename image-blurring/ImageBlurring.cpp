#include "include/common.h"
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
// OpenCL includes
#include <CL/cl.h>

#define STB_IMAGE_IMPLEMENTATION  // needed to enable the STB image implementations
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION  // needed to enable the STB image write implementations
#include "include/stb_image_write.h"

#define NUM_CHANNEL 4

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
    int width;
    int height;
    int bpp;  // bytes per pixel (4)
    uint8_t* rgb_image = stbi_load("./include/bear.png", &width, &height, &bpp, STBI_rgb_alpha);
    if (rgb_image == nullptr)
    {
        std::cerr << "stbi_load returned null!" << std::endl;
        clReleaseProgram(program);
        clReleaseContext(context);
        return 0;
    }
    // print image info
    std::cout << "image loaded with width: " << width << ", height: " << height << ", bpp: " << bpp << std::endl;

    std::vector<uint8_t> host_data(rgb_image, rgb_image + width * height * NUM_CHANNEL);  // do it for consistency, though it's not required!
    const size_t length = host_data.size();
    const size_t size_in_byte = length * sizeof(decltype(host_data.at(0)));
    stbi_image_free(rgb_image);  // rgb_image can be freed because host_data has a copy of it

    // prepare Gaussian kernel weights
    // weights follow the Gaussian distribution and can be generated automatically
    // since this is not the focus of this code, numbers are hard-coded
    // vector below is the first row/column of the Gaussian Blur matrix
    const std::vector<float> gaussian_kernel_weights = {0.25f, 0.5f, 0.25f};
    const size_t gaussian_kernel_size = gaussian_kernel_weights.size();

    // create buffer(s)
    cl_mem device_data = clCreateBuffer(context, CL_MEM_READ_WRITE, size_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");

    cl_mem device_kernel_weights = clCreateBuffer(context, CL_MEM_READ_ONLY, gaussian_kernel_size*sizeof(float), NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");

    // transfer data to GPU
    clEnqueueWriteBuffer(queue, device_data, CL_TRUE, 0, size_in_byte, host_data.data(), 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, device_kernel_weights, CL_TRUE, 0, gaussian_kernel_size*sizeof(float), gaussian_kernel_weights.data(), 0, NULL, NULL);

    // build kernel(s)
    cl_kernel kernel_image_blurring = clCreateKernel(program, "imageBlurring", &err);
    CHECK_CL_ERROR(err, "Couldn't create the image blurring kernel");

    // set kernel args
    err = clSetKernelArg(kernel_image_blurring, 0, sizeof(cl_mem), &device_data);
    CHECK_CL_ERROR(err, "Couldn't set arg 1");
    clSetKernelArg(kernel_image_blurring, 1, sizeof(width), &width);
    CHECK_CL_ERROR(err, "Couldn't set arg 2");
    clSetKernelArg(kernel_image_blurring, 2, sizeof(height), &height);
    CHECK_CL_ERROR(err, "Couldn't set arg 3");
    clSetKernelArg(kernel_image_blurring, 3, sizeof(cl_mem), &device_kernel_weights);
    CHECK_CL_ERROR(err, "Couldn't set arg 4");
    clSetKernelArg(kernel_image_blurring, 4, sizeof(gaussian_kernel_size), &gaussian_kernel_size);
    CHECK_CL_ERROR(err, "Couldn't set arg 5");

    // print max available threads per dimension for the selected device
    size_t max_work_item_size[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_size), max_work_item_size, NULL);
    std::cout << "Max threads available in X: " << max_work_item_size[0] << ", Y: " << max_work_item_size[1] << ", Z: " << max_work_item_size[2] << std::endl;

    // print max local memory size
    // Get the maximum local memory size per workgroup
    size_t max_local_mem_size;
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &max_local_mem_size, NULL);
    std::cout << "Max local memory size in bytes is: " << max_local_mem_size << std::endl;


    // set global and local sizes (grid and block sizes)
    // MAX number of threads is 512, so setting each dimention to 16 since 16x16 < 512
    size_t global_size[] = {16u, 16u};
    size_t local_size[] = {16u, 16u};

    // enqueue the kernel for execution
    err = clEnqueueNDRangeKernel(queue, kernel_image_blurring, 2, NULL, &global_size[0], &local_size[0], 0, NULL, NULL);
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
    const std::string output_file_name = "out.png";
    stbi_write_png(output_file_name.c_str(), width, height, NUM_CHANNEL, host_data.data(), width * NUM_CHANNEL);

    return 0;
}