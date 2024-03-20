#include "include/Data.h"
#include "include/common.h"
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
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
    std::vector<float> host_in_data = data;
    const size_t size_data_in_byte = layers[0] * sizeof(decltype(host_in_data.at(0)));
    const size_t num_layers = layers.size();
    const size_t all_outputs_size = std::accumulate(layers.cbegin() + 1, layers.cend(), 0u);
    std::vector<float> host_all_outputs(all_outputs_size);

    // create buffer for in data
    cl_mem device_in_data = clCreateBuffer(context, CL_MEM_READ_WRITE, size_data_in_byte, NULL, &err);
    CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");

    cl_mem device_out_data; // to be populated in the for loop
    cl_mem device_weights; // to be populated in the for loop

    // transfer data to GPU
    clEnqueueWriteBuffer(queue, device_in_data, CL_TRUE, 0, size_data_in_byte, host_in_data.data(), 0, NULL, NULL);

    // build kernel(s)
    cl_kernel kernel_forwardPass = clCreateKernel(program, "forwardPass", &err);
    CHECK_CL_ERROR(err, "Couldn't create the forwardPass kernel");

    // set global and local sizes (grid and block sizes)
    size_t global_size = 32u;
    size_t local_size = 32u;
    size_t all_outputs_index_to_start = 0u;
    size_t weights_index_to_start = 0u;

    for (auto i = 0u; i < num_layers - 1; ++i)
    {
        // create buffer(s)
        device_out_data = clCreateBuffer(context, CL_MEM_READ_WRITE, layers[i+1] * sizeof(float), NULL, &err);
        device_weights = clCreateBuffer(context, CL_MEM_READ_ONLY, layers[i] * layers[i+1] * sizeof(float), NULL, &err);
        CHECK_CL_ERROR(err, "Couldn't create the buffer on the GPU");
        // transfer data to GPU
        clEnqueueWriteBuffer(queue, device_weights, CL_TRUE, 0, layers[i] * layers[i+1] * sizeof(float), weights.data() + weights_index_to_start, 0, NULL, NULL);
        weights_index_to_start += layers[i] * layers[i+1];

        // set kernel args
        err = clSetKernelArg(kernel_forwardPass, 0, sizeof(cl_mem), &device_in_data);
        CHECK_CL_ERROR(err, "Couldn't set arg 1");
        err = clSetKernelArg(kernel_forwardPass, 1, sizeof(cl_mem), &device_weights);
        CHECK_CL_ERROR(err, "Couldn't set arg 2");
        err = clSetKernelArg(kernel_forwardPass, 2, sizeof(cl_mem), &device_out_data);
        CHECK_CL_ERROR(err, "Couldn't set arg 3");
        err = clSetKernelArg(kernel_forwardPass, 3, sizeof(layers[i]), &layers[i]);
        CHECK_CL_ERROR(err, "Couldn't set arg 4");
        err = clSetKernelArg(kernel_forwardPass, 4, sizeof(layers[i+1]), &layers[i+1]);
        CHECK_CL_ERROR(err, "Couldn't set arg 5");

        // enqueue the kernel for execution
        err = clEnqueueNDRangeKernel(queue, kernel_forwardPass, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
        CHECK_CL_ERROR(err, "Couldn't launch the forwardPass kernel");

        // wait until execution of the kernel is over
        err = clFinish(queue);
        CHECK_CL_ERROR(err, "Couldn't empty the queue");

        // read the kernel's output (cluster ids), there is no need to read back data as it's unchanged
        clEnqueueReadBuffer(queue, device_out_data, CL_TRUE, 0, layers[i+1] * sizeof(float), host_all_outputs.data() + all_outputs_index_to_start, 0, NULL, NULL);
        all_outputs_index_to_start += layers[i+1];

        // wait until data is compeletely read from device 
        err = clFinish(queue);
        CHECK_CL_ERROR(err, "Couldn't empty the queue");

        // update device_in_data
        device_in_data = device_out_data;
    }

    // Release resources
    clReleaseMemObject(device_in_data);
    clReleaseMemObject(device_out_data);
    clReleaseKernel(kernel_forwardPass);
    clReleaseProgram(program);
    clReleaseContext(context);
    
    // write the result to disk
    const std::string output_file_name = "out.txt";
    std::ofstream out(output_file_name);
    if (out.is_open())
    {
        // copy the content of host_data to disk
        std::copy(host_all_outputs.cbegin(), host_all_outputs.cend(), std::ostream_iterator<double>(out, " "));
        out.close();
    }
    else
    {
        std::cerr << "Couldn't open the output file" << std::endl;
    }

    return 0;
}