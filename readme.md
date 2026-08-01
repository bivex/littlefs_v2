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

Examples & Projects
-------------------
1. **Asset Package Vault Demo (1,000 files)**: [`src/example/asset_vault_example.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/asset_vault_example.cpp)
   Demonstrates creating, packing, directory scanning, and batch-updating 1,000 small files (JSON configs, textures, audio FX descriptors) inside a single `.vfs` package container.
2. **Unit Test Suite**: [`src/example/vfs_tests.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/vfs_tests.cpp)
   Complete unit test suite with 46 assertions passing across Memory & File backends.
3. **Architecture Documentation**: [`docs/vfs_value_and_architecture.md`](file:///Volumes/External/Code/f/littlefs_v2/docs/vfs_value_and_architecture.md)
   Commercial & technical value breakdown.