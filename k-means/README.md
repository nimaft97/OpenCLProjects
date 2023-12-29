# K-means Clustering

This repository provides an implementation of the K-Means clustering algorithm, a powerful and widely used technique in machine learning and data analysis.

## K-means Clustering Overview

K-Means is an unsupervised learning algorithm that partitions a dataset into K distinct, non-overlapping subsets (clusters). Each data point is assigned to the cluster with the nearest mean, and the algorithm iteratively refines the cluster assignments to converge to a stable solution.

## Atomic Operations for Floats

The K-Means algorithm relies on atomic operations for floats, which are not directly supported in OpenCL. To overcome this limitation, the implementation cleverly employs Compare-And-Swap (CAS) loops and custom atomic addition functions for floats. The `atomic_cmpxchg_f32_local` function atomically updates a float using a CAS loop, and the `atomic_add_f32_local` function performs atomic addition for floats. These functions utilize unions to interchangeably use variables as uint and float, enabling the necessary atomic operations.

An alternative approach to handling atomic operations for floating-point values involves leveraging sub-group operations, particularly the sub_group_reduce_add function. Sub-groups are smaller units within a work-group that can perform operations more efficiently than the entire work-group. The sub_group_reduce_add function is designed to efficiently compute the sum of values across a sub-group. However, there are certain limitations to this approach. The size of the sub-group is not controlable, meaning that it cannot be deterministically used in the implementation. Otherwise, each thread (work-item) could have its own copy of clusters and their members, and in the end, `sub_group_reduce_add` would help put together all pieces of information and update the local memory only once.

For more details, refer to [Khronos webpage for Intel sub-group optimized operations](https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups.html).

### Assumptions

- There is only one block of threads (work group) executing the kernel because there is no point in having work-items distributed between multiple work-groups for this problem.
- The input data can be stored in the shared memory, which is device dependent.

## Getting Started

To use the k-means clustering implementation in your project, follow the steps outlined in the README. You'll find instructions on building, configuring, and integrating the code into your OpenCL-enabled application.

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