# Image Blurring


Image blurring, also known as image smoothing or image softening, is a fundamental technique in image processing aimed at reducing noise and detail in an image. By averaging the pixel values in the vicinity of each pixel, blurring creates a smoother appearance, effectively suppressing high-frequency components while preserving the overall structure and important features of the image.

Gaussian image blurring is a specific method of image smoothing that utilizes a Gaussian filter kernel to achieve the blurring effect. Unlike simpler averaging techniques, such as box blur, Gaussian blurring applies a weighted average to the pixels surrounding each target pixel, with the weights determined by a Gaussian distribution. This distribution assigns higher weights to pixels closer to the target pixel and lower weights to those farther away, resulting in a smooth, bell-shaped falloff of influence.

## GPU Gaussian Image Blurring Overview


The GPU-accelerated method employed in this Gaussian Image Blurring approach has been finely tuned by dividing the grid into separate horizontal and vertical 1D convolutions. This strategic decomposition enhances data access efficiency and minimizes redundant read operations. To expedite data retrieval, the input image is segmented into smaller sections that fit within the local memory. While this optimization accelerates processing, it introduces an additional layer of approximation since the input image is fragmented into smaller, distinct images before reassembly post-kernel execution.

For more information on the technique used in this implementation and other common techniques for image blurring, visit [intel's website](https://www.intel.com/content/www/us/en/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html).

## Example

Input image:
![sample-input](https://github.com/nimaft97/OpenCLProjects/blob/main/image-blurring/images/bear-in.png)

Output image (kernel size = 3):
![sample-output](https://github.com/nimaft97/OpenCLProjects/blob/main/image-blurring/images/bear-out.png)


### Assumptions

- Global array is big enough that the input image can be completely transferred to GPU
- Input image as RGBA so it's read as an array of 4 uchars

## Getting Started

To use the image blurring implementation in your project, follow the steps outlined in the README. You'll find instructions on building, configuring, and integrating the code into your OpenCL-enabled application.

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