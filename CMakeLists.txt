cmake_minimum_required(VERSION 3.20)
project(littlefs_v2 
    VERSION 2.1.0 
    DESCRIPTION "High-Performance Thread-Safe Virtual File System with CTZ Skip-Lists, Page Cache & Decorators"
    HOMEPAGE_URL "https://github.com/jnastarot/littlefs_v2"
    LANGUAGES CXX C
)

# Standard C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Options
option(LITTLEFS_BUILD_TESTS "Build test runner suite" ON)
option(LITTLEFS_BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(LITTLEFS_BUILD_EXAMPLES "Build demo and scanner examples" ON)
option(LITTLEFS_ENABLE_THREADSAFETY "Enable thread-safe mutex protection" ON)

# Core LittleFS v2 Source Files
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
    src/example/lfs_interface.cpp
)

# Static Library Target
add_library(littlefs_v2 STATIC ${LFS_SOURCES})
add_library(littlefs_v2::littlefs_v2 ALIAS littlefs_v2)

target_compile_features(littlefs_v2 PUBLIC cxx_std_20)

target_include_directories(littlefs_v2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/example>
    $<INSTALL_INTERFACE:include>
)

if(LITTLEFS_ENABLE_THREADSAFETY)
    target_compile_definitions(littlefs_v2 PUBLIC LFS_THREADSAFE)
endif()

# Compiler Warning Flags
if(MSVC)
    target_compile_options(littlefs_v2 PRIVATE /W4 /permissive-)
else()
    target_compile_options(littlefs_v2 PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Executables & Tools
if(LITTLEFS_BUILD_EXAMPLES)
    # Example application
    add_executable(example src/example/example.cpp)
    target_link_libraries(example PRIVATE littlefs_v2)

    # DPX Codebase Scanner & Ingest tool
    add_executable(dpx_scanner src/example/dpx_scanner.cpp)
    target_link_libraries(dpx_scanner PRIVATE littlefs_v2)
endif()

if(LITTLEFS_BUILD_BENCHMARKS)
    # Throughput, IOPS and Latency Benchmark
    add_executable(benchmark_throughput src/benchmarks/benchmark_throughput.cpp)
    target_link_libraries(benchmark_throughput PRIVATE littlefs_v2)

    # Small Files Problem Benchmark
    add_executable(benchmark_small_files src/benchmarks/benchmark_small_files.cpp)
    target_link_libraries(benchmark_small_files PRIVATE littlefs_v2)
endif()

if(LITTLEFS_BUILD_TESTS)
    # Test Runner
    add_executable(tests_runner src/tests/test_main.cpp)
    target_link_libraries(tests_runner PRIVATE littlefs_v2)

    # CTest Integration
    enable_testing()
    add_test(NAME UnitAndChaosTestSuite COMMAND tests_runner)
    if(TARGET example)
        add_test(NAME BasicBenchmarkExample COMMAND example)
    endif()
endif()

# Installation & CMake Package Configuration
include(GNUInstallDirs)
install(TARGETS littlefs_v2
    EXPORT littlefs_v2Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES src/example/lfs_interface.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT littlefs_v2Targets
    FILE littlefs_v2Targets.cmake
    NAMESPACE littlefs_v2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/littlefs_v2
)
