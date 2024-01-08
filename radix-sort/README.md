# Radix Sort

This repository includes a parallel implementation of the Radix Sort algorithm using OpenCL for GPU acceleration. Radix Sort is a linear-time sorting algorithm that works by distributing elements into buckets based on their individual digits. 

## Radix Sort Overview

The implementation involves bitwise extraction of digits, bucketing, local sorting within each bucket, and concatenation steps. Each iteration processes a different bit position (digit), contributing to the overall efficiency of the parallel sorting process.

The provided kernel performs radix sort on an array of integers. It uses shared memory to efficiently perform parallel counting sort for each digit. The algorithm is structured with a series of steps, including counting occurrences per bucket, performing prefix sum using local memory, and placing the numbers in order.

In the OpenCL kernel implemented here, cumulative sums are also calculated in parallel using Prefix-Sum (implemented [here](https://github.com/nimaft97/OpenCLProjects/tree/main/prefix-scan)). Since in OpenCL, dynamic parallelization (work-items being able to launch other kernels) is not yet possible, Prefix-Scan has been integrated into the same kernel to guarantee its parallel execution. 

### Assumptions

- There is only one block of threads (work group) executing the kernel because there is no point in having work-items distributed between multiple work-groups for this problem.
- The input data can be stored in the shared memory, which is device dependent.
- The input data consists of unsigned integers

## Getting Started

To use the radix sort implementation in your project, follow the steps outlined in the README. You'll find instructions on building, configuring, and integrating the code into your OpenCL-enabled application.

### Installing OpenCL
It is assumed that OpenCL is already installed. The commands used in the OpenCL installation process are listed below:

- Follow the steps in [OpenCL Windows Installation Manual](https://github.com/KhronosGroup/OpenCL-Guide/blob/main/chapters/getting_started_windows.md)
- (Windows users) Open a Windows Power Shell with Administrative provileges

       1. `if (-not (Test-Path ~\Documents\PowerShell)) { New-Item -Type Directory ~\Documents\PowerShell }`
       2. `Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll" >> $PROFILE`
       3. `function devshell { Enter-VsDevShell -InstallPath "C:\Program Files\Microsoft Visual Studio\2022\Community\" -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -no_logo" }  ` 
       4. `devshell -VsInstallPath 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools' -DevCmdArguments '-arch=x64 -no_logo'`

### Compilitation

Go to the main dircteroy of the project and do the following:

#### Command-line

- `cl.exe /nologo /TC /W4 /DCL_TARGET_OPENCL_VERSION=100 /IC:\Users\nimaf\OneDrive\Desktop\IE\projects\opencl-proj\OpenCL-SDK\install\include\ Main.c /Fe:HelloOpenCL /link /LIBPATH:C:\Users\nimaf\OneDrive\Desktop\IE\projects\opencl-proj\OpenCL-SDK\install\lib OpenCL.lib`
       
#### CMake

- Go to where the CMake exists
- `cmake -A x64 -S . -B .\build -D CMAKE_PREFIX_PATH=C:\Users\nimaf\OneDrive\Desktop\IE\projects\opencl-proj\OpenCL-SDK\install\` 


### Building

- `cmake --build .\build --config <Release/Debug>`
- Find the executable under builds in the folder created for the requested config (Release/Debug)                           