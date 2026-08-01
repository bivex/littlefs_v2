littlefs v2
====

Refactored [littlefs](https://github.com/littlefs-project/littlefs) with extensions for use as a high-performance virtual filesystem.

Features
--------
- Refactored C++ codebase
- Support for file size up to `0x7FFFFFFFFFFFFFFF`
- Dynamic disk auto-grow without remounting
- 2 backends: in-memory backend and file-backed virtual filesystem
- **High-Performance VFS Page Cache Layer**:
  - $O(1)$ random seek, tell, and read in RAM
  - Batched flush for random writes (reduces Write Amplification Factor and accelerates random writes by up to 100x+)
- Cross-platform support (Windows / macOS / Linux with MSVC, Clang, GCC)

Performance Benchmark
---------------------
*Tested on 512 KB virtual file:*
- **Sequential Write (512 KB)**: `2 ms`
- **Random Seek & Read (5,000 ops)**: `0 ms` ($O(1)$ in-memory lookups)
- **Random Overwrite (1,000 ops + Batched Flush)**: `17 ms`
- **Data Integrity Verification**: `PASSED (100% OK)`

Usage Example
-------------
See [`src/example/example.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/example.cpp) for a full usage and benchmark example.