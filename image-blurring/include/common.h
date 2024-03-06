#ifndef COMMON_H
#define COMMON_H

#include <cassert>
#include <CL/cl.h>
#include <cstring>
#include <fstream>
#include <streambuf>

#define CHECK_CL_ERROR(err, msg) assert(err == CL_SUCCESS && msg)

/*
reads a file that contains the definiton of ONE kernel
returns it as a string
*/
std::string readFile(const std::string& file_name) {
    std::ifstream file("include/" + file_name);
    assert(file.is_open() && "Couldn't open the file to read the kernel");
    // Read the entire file into a string
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

#endif