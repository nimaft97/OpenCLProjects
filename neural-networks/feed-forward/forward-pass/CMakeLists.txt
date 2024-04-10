cmake_minimum_required(VERSION 3.4)
project(OpenCLProject)
find_package(OpenCL CONFIG REQUIRED)
add_executable(${PROJECT_NAME} forwardPass.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD 17
                                                 CMAKE_CXX_STANDARD_REQUIRED ON
                                                 CMAKE_CXX_EXTENSIONS OFF)
set_source_files_properties(forwardPass.cpp PROPERTIES
    COMPILE_FLAGS "/std:c++17"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE CL_TARGET_OPENCL_VERSION=100)

# introduce dependency on Google GTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# enable unit testing
enable_testing()
add_executable(
  unittest
  unittest.cc
)
target_link_libraries(
  unittest
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(unittest)
