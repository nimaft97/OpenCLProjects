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
    std::vector<float> host_data_m1 = flatten2D<float>(matrix_1);  // flatten and check if dim1 and dim2 are positive
    const size_t dim1_1 = matrix_1.size();
    const size_t dim1_2 = matrix_1[0].size();

    std::vector<float> host_data_m2 = flatten2D<float>(matrix_2);  // flatten and check if dim1 and dim2 are positive
    const size_t dim2_1 = matrix_2.size();
    const size_t dim2_2 = matrix_2[0].size();

    assert(dim1_2 == dim2_1 && "multiplication is not possible. Dimensions do not match!");

    const size_t size_m1_in_byte = dim1_1 * dim1_2 * sizeof(decltype(host_data_m1.at(0)));
    const size_t size_m2_in_byte = dim2_1 * dim2_2 * sizeof(decltype(host_data_m2.at(0)));
    std::vector<float> host_data_m3(dim1_1 * dim2_2);
    const size_t size_m3_in_byte = dim1_1 * dim2_2 * sizeof(decltype(host_data_m1.at(0)));

    // create buffer(s)
    // data is read-only
    cl_mem device_data_m1 = clCreateBuffer(context, CL_MEM_READ_ONLY, size_m1_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");
    cl_mem device_data_m2 = clCreateBuffer(context, CL_MEM_READ_WRITE, size_m2_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");
    // cluster ids is write-only, so there is no need to write the content from host to device
    cl_mem device_data_m3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size_m3_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");

    // transfer data to GPU
    clEnqueueWriteBuffer(queue, device_data_m1, CL_TRUE, 0, size_m1_in_byte, host_data_m1.data(), 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, device_data_m2, CL_TRUE, 0, size_m2_in_byte, host_data_m2.data(), 0, NULL, NULL);
    // build kernel(s)
    cl_kernel kernel_matrix_tran = clCreateKernel(program, "matrixTranspose", &err);
    CHECK_CL_ERROR(err, "Couldn't create the matrixTranspose kernel");
    cl_kernel kernel_matrix_mul = clCreateKernel(program, "matrixMul", &err);
    CHECK_CL_ERROR(err, "Couldn't create the matrixMul kernel");

    // set kernel args
    err = clSetKernelArg(kernel_matrix_tran, 0, sizeof(cl_mem), &device_data_m2);
    CHECK_CL_ERROR(err, "Couldn't set arg 1");
    clSetKernelArg(kernel_matrix_tran, 1, sizeof(dim2_1), &dim2_1);
    CHECK_CL_ERROR(err, "Couldn't set arg 2");
    clSetKernelArg(kernel_matrix_tran, 2, sizeof(dim2_2), &dim2_2);
    CHECK_CL_ERROR(err, "Couldn't set arg 3");

    // set kernel args
    err = clSetKernelArg(kernel_matrix_mul, 0, sizeof(cl_mem), &device_data_m1);
    CHECK_CL_ERROR(err, "Couldn't set arg 1");
    err = clSetKernelArg(kernel_matrix_mul, 1, sizeof(cl_mem), &device_data_m2);
    CHECK_CL_ERROR(err, "Couldn't set arg 2");
    err = clSetKernelArg(kernel_matrix_mul, 2, sizeof(cl_mem), &device_data_m3);
    CHECK_CL_ERROR(err, "Couldn't set arg 3");
    clSetKernelArg(kernel_matrix_mul, 3, sizeof(dim1_1), &dim1_1);
    CHECK_CL_ERROR(err, "Couldn't set arg 4");
    clSetKernelArg(kernel_matrix_mul, 4, sizeof(dim1_2), &dim1_2);
    CHECK_CL_ERROR(err, "Couldn't set arg 5");
    clSetKernelArg(kernel_matrix_mul, 5, sizeof(dim2_2), &dim2_2);
    CHECK_CL_ERROR(err, "Couldn't set arg 6");

    // set global and local sizes (grid and block sizes)
    size_t global_size = 32u;
    size_t local_size = 32u;

    // enqueue the kernel for execution
    err = clEnqueueNDRangeKernel(queue, kernel_matrix_tran, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "Couldn't launch the matrixTranspose kernel");

    err = clEnqueueNDRangeKernel(queue, kernel_matrix_mul, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "Couldn't launch the matrixMul kernel");

    // wait until execution of the kernel is over
    err = clFinish(queue);
    CHECK_CL_ERROR(err, "Couldn't empty the queue");

    // read the kernel's output (cluster ids), there is no need to read back data as it's unchanged
    clEnqueueReadBuffer(queue, device_data_m3, CL_TRUE, 0, size_m3_in_byte, host_data_m3.data(), 0, NULL, NULL);

    // wait until data is compeletely read from device 
    err = clFinish(queue);
    CHECK_CL_ERROR(err, "Couldn't empty the queue");

    // Release resources
    clReleaseMemObject(device_data_m1);
    clReleaseMemObject(device_data_m2);
    clReleaseMemObject(device_data_m3);
    clReleaseKernel(kernel_matrix_mul);
    clReleaseProgram(program);
    clReleaseContext(context);
    
    // write the result to disk
    const std::string output_file_name = "out.txt";
    std::ofstream out(output_file_name);
    if (out.is_open())
    {
        // copy the content of host_data to disk
        for (auto i = 0u; i < dim1_1; ++i)
        {
            std::copy(host_data_m3.cbegin() + i * dim2_2,
                      host_data_m3.cbegin() + (i+1) * dim2_2,
                      std::ostream_iterator<float>(out, " "));
        }
        out.close();
    }
    else
    {
        std::cerr << "Couldn't open the output file" << std::endl;
    }

    return 0;
}