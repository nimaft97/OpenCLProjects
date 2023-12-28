# Prefix Scan

This repository contains an implementation of the prefix scan algorithm, a fundamental operation in parallel computing. The prefix scan, also known as a parallel scan or inclusive scan, is widely used in various domains such as image processing, computational biology, and simulations.

## Prefix Scan Overview

The prefix scan efficiently computes the cumulative sum of elements in an input array in a parallel fashion. It is a building block for many parallel algorithms and can significantly improve the performance of certain operations.

For more details, refer to [NVidia's GPU Gems3](https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda)

### Assumptions

- There is only one block of threads (work group) executing the kernel because there is no point in having work-items distributed between multiple work-groups for this problem.
- The input data can be stored in the shared memory, which is device dependent.

## Getting Started

To use the prefix scan implementation in your project, follow the steps outlined in the README. You'll find instructions on building, configuring, and integrating the code into your OpenCL-enabled application.

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