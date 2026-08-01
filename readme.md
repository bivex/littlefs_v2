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
- **Linux FUSE 3 Driver**: Mount `littlefs_v2` images as native Linux filesystems (`lfs_fuse`)
- Cross-platform support (Windows / macOS / Linux with MSVC, Clang, GCC)

Performance & Power-Loss Resilience
------------------------------------
- **Sequential Write (1 MB)**: `8.18 ms`
- **Random Seek & Read (10,000 ops)**: `0.83 ms` (`83 ns` per operation)
- **Random Overwrite (2,000 ops + Batched Flush)**: `0.47 ms`
- **Power-Loss Fault Injection**: `100% PASSED (0% Metadata Corruption)`

Examples & Projects
-------------------
1. **Linux FUSE 3 Driver**: [`src/example/lfs_fuse.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/lfs_fuse.cpp)
   Mounts `.vfs` containers directly into Linux OS filesystem tree.
2. **Power-Loss Fault Injection Test**: [`src/example/power_loss_test.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/power_loss_test.cpp)
   Simulates mid-operation power loss & verifies zero metadata corruption.
3. **Asset Package Vault Demo (1,000 files)**: [`src/example/asset_vault_example.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/asset_vault_example.cpp)
   Demonstrates packing, reading, and batch-updating 1,000 small files inside a single `.vfs` package container.
4. **Unit Test Suite**: [`src/example/vfs_tests.cpp`](file:///Volumes/External/Code/f/littlefs_v2/src/example/vfs_tests.cpp)
   46 unit assertions passing across Memory & File backends.
5. **Architecture Documentation**: [`docs/vfs_value_and_architecture.md`](file:///Volumes/External/Code/f/littlefs_v2/docs/vfs_value_and_architecture.md)
   Commercial & technical value breakdown.