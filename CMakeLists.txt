cmake_minimum_required(VERSION 3.15)
project(littlefs_v2 VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(include src/example)

# Core LittleFS v2 source files
set(LFS_SOURCES
    src/littlefs_v2/lfs_allocator.cpp
    src/littlefs_v2/lfs_commit.cpp
    src/littlefs_v2/lfs_device.cpp
    src/littlefs_v2/lfs_directory.cpp
    src/littlefs_v2/lfs_file.cpp
    src/littlefs_v2/lfs_file_index.cpp
    src/littlefs_v2/lfs_general.cpp
    src/littlefs_v2/lfs_metadata.cpp
    src/littlefs_v2/lfs_operations.cpp
    src/littlefs_v2/lfs_toplevel.cpp
)

# Static library
add_library(littlefs_v2 STATIC ${LFS_SOURCES} src/example/lfs_interface.cpp)
target_include_directories(littlefs_v2 PUBLIC include src/example)
target_compile_definitions(littlefs_v2 PUBLIC LFS_THREADSAFE)

# Example executable
add_executable(example src/example/example.cpp)
target_link_libraries(example PRIVATE littlefs_v2)

# Test Runner executable
add_executable(tests_runner src/tests/test_main.cpp)
target_link_libraries(tests_runner PRIVATE littlefs_v2)

# Benchmark Throughput executable
add_executable(benchmark_throughput src/benchmarks/benchmark_throughput.cpp)
target_link_libraries(benchmark_throughput PRIVATE littlefs_v2)

# Enable CTest
enable_testing()
add_test(NAME LittleFSTests COMMAND tests_runner)
