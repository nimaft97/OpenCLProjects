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

/*
flattens a 2D vector of dimension mxn and
returns a 1D vector of length m * n
*/
template <typename T>
std::vector<T> flatten2D(const std::vector<std::vector<T>>& in_vec)
{
    std::vector<T> ret;
    const auto dim1 = in_vec.size();
    assert(dim1 > 0 && "2D array must have a positive first dimension");
    const auto dim2 = in_vec[0].size();
    assert(dim2 > 0 && "2D array must have a positive second dimension");
    ret.reserve(dim1 * dim2);

    for (const auto& vec : in_vec)
    {
        ret.insert(ret.end(), vec.cbegin(), vec.cend());
    }

    return ret;
}

#endif