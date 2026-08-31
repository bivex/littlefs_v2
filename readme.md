# littlefs v2 — High-Performance Thread-Safe Virtual File System (VFS)

`littlefs_v2` — a modernized, modular C++ implementation of [ARM littlefs](https://github.com/littlefs-project/littlefs) extended with **Thread-Safe VFS**, **High-Performance Page Cache**, **Pluggable Block Device Decorators (Crypto & Fault-Injection)**, **Linux FUSE 3 driver**, and **Ninja** build system.

---

## 🌟 Key Features & Architecture

* **Modular C++ Architecture**: Decoupled the original monolithic C codebase into dedicated, maintainable modules (`lfs_allocator`, `lfs_commit`, `lfs_directory`, `lfs_file`, `lfs_file_index`, `lfs_metadata`, `lfs_operations`, `lfs_toplevel`).
* **Thread-Safety & Locking**: Built-in recursive mutex locking across all VFS and file handle operations (`LFS_THREADSAFE`), ensuring full safety in concurrent multi-threaded environments.
* **High-Performance Page Cache Layer**:
  * $O(1)$ random seek, read, and size calculation in memory.
  * **Batched Flush**: Aggregates thousands of random in-memory writes into sequential disk commits, avoiding CTZ skip-list relocation overhead.
* **Modern Error Handling & RAII**:
  * `fs::Result<T, ErrorCode>` type eliminating raw negative error codes and enabling ergonomic value/error checking.
  * Move-only RAII `fs::FileHandle` that guarantees automatic `flush()` and `close()` upon destruction.
* **64-bit Sizes & Offsets**: All block numbers, sizes, and file offsets upgraded to 64-bit integers (`uint64_t`), enabling file sizes up to `0x7FFFFFFFFFFFFFFF` (8 EiB).
* **Dynamic Disk Auto-Grow**: Automatic storage capacity expansion on demand (`lfs_fs_grow`) without unmounting or remounting.
* **Pluggable Block Device & Decorator Layer**:
  * `fs::IBlockDevice` abstraction interface.
  * `MemoryBlockDevice`: Ultra-fast in-RAM virtual filesystem.
  * `FileBlockDevice`: Host-file container acting as a virtual disk.
  * `CryptoBlockDevice`: Transparent per-block cryptographic scrambler / encryption decorator.
  * `FaultInjectBlockDevice`: Chaos simulator for write errors and sudden power-loss events.
* **Linux FUSE 3 Driver**: Mount virtual `.vfs` containers directly into Linux filesystem namespace (`lfs_fuse`).

---

## 📊 Performance Benchmarks (Page Cache & VFS)

*Measurements on 1 MB file (1,048,576 bytes):*

| Operation | Volume / Count | Elapsed Time | Throughput |
| :--- | :---: | :---: | :---: |
| **Sequential Write** | 1 MB | **`8.18 ms`** | `128 MB/s` |
| **Random Seek & Read (RAM)** | 10,000 ops | **`0.83 ms`** | **`83 ns / op`** |
| **Random Overwrite (Page Cache)** | 2,000 ops | **`0.17 ms`** | **`88 ns / op`** |
| **Batched Flush to Disk** | 2,000 writes | **`0.47 ms`** | `< 0.5 ms` |
| **Directory Scan** | 1,000 files | **`15.33 ms`** | `15 us / file` |

---

## ⚡ Power-Loss Resilience & Chaos Tests

* **Unit & Concurrency Test Suite (`src/tests/test_main.cpp`)**: Verifies CRUD, Auto-Grow, 8-thread concurrent read/write stress, and Crypto Block Device.
* **Fault Injection & Power-Loss Recovery**: **`0% Metadata Corruption`**. Injected write failures recover cleanly to the previous consistent revision state via LittleFS dual-block logging and CRC verification.

---

## 🚀 Quick Start / Usage Example

```cpp
#include <iostream>
#include "lfs_interface.h"

int main() {
    // 1. Create an in-memory or file-backed VFS
    auto dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 8);
    auto vfs_res = fs::createVFSWithDevice(dev);
    if (!vfs_res.has_value()) {
        std::cerr << "Failed to create VFS" << std::endl;
        return 1;
    }
    auto vfs = vfs_res.value();

    // 2. Open, write, and read using modern RAII FileHandle
    auto file_res = vfs->open("hello.txt", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
    if (file_res.has_value()) {
        fs::FileHandle file(file_res.value());
        
        const std::string text = "Hello LittleFS v2 with Thread Safety & Modern C++!";
        file->write(text.c_str(), text.size());
        file->flush();

        file->seek(0, fs::IFileObject::kSeekSet);
        std::vector<char> buf(text.size() + 1, 0);
        file->read(buf.data(), text.size());
        std::cout << "Read from VFS: " << buf.data() << std::endl;
    }

    // 3. List directory entries
    auto dir_res = vfs->listDir("/");
    if (dir_res.has_value()) {
        for (const auto& entry : dir_res.value()) {
            std::cout << " - " << entry.getPath() << " (" << entry.getSize() << " bytes)\n";
        }
    }

    return 0;
}
```

---

## 🛠 Build & Test (Ninja)

The project uses CMake with **Ninja** build system and presets:

```bash
# Option 1: Using CMake Presets (Ninja)
cmake --preset default
cmake --build --preset default

# Option 2: Direct Ninja invocation
cmake -B build -G Ninja
ninja -C build

# Run example
./build/example

# Run test suite
./build/tests_runner
```

---

## 🐧 Linux FUSE 3 Driver Build & Mount

```bash
# 1. Install FUSE 3 dependencies (Ubuntu / Debian)
sudo apt-get update && sudo apt-get install -y libfuse3-dev build-essential pkg-config

# 2. Compile FUSE driver
g++ -std=c++20 -O2 -Iinclude -Isrc/example \
    src/littlefs_v2/*.cpp src/example/lfs_interface.cpp src/example/lfs_fuse.cpp \
    -D_FILE_OFFSET_BITS=64 $(pkg-config --cflags --libs fuse3) -o littlefs_fuse

# 3. Mount container
mkdir -p /mnt/my_vfs
./littlefs_fuse /mnt/my_vfs

# 4. Use standard Linux commands
echo "Hello from Linux terminal!" > /mnt/my_vfs/test.txt
cat /mnt/my_vfs/test.txt

# 5. Unmount
fusermount3 -u /mnt/my_vfs
```

---

## 📁 Project Structure

```
littlefs_v2/
├── CMakeLists.txt             # Modern CMake build system
├── CMakePresets.json          # Ninja presets (default, debug)
├── include/
│   ├── lfs.h                  # Core LittleFS definitions and configurations
│   ├── lfs_utility.h          # Bit operations (ctz, clz, popcount), logging, and assertions
│   ├── lfs_result.h           # Result<T, ErrorCode> error handling
│   └── lfs_block_device.h     # IBlockDevice, Memory, File, Crypto, and Fault-Injection devices
├── src/
│   ├── littlefs_v2/           # Modular core LittleFS implementation
│   │   ├── lfs_allocator.cpp    # Free space lookahead buffer and block allocation
│   │   ├── lfs_commit.cpp       # Metadata commit logs, compaction, and splitting
│   │   ├── lfs_device.cpp       # Block device I/O abstraction and caching
│   │   ├── lfs_directory.cpp    # Directory management and path traversal
│   │   ├── lfs_file.cpp         # File I/O operations (read, write, seek, truncate)
│   │   ├── lfs_file_index.cpp   # CTZ skip-list index operations
│   │   ├── lfs_general.cpp      # Mount, format, superblock, and fs traversal
│   │   ├── lfs_metadata.cpp     # Tag/slice operations and attribute fetching
│   │   ├── lfs_operations.cpp   # File stat, remove, rename, and fs growth
│   │   └── lfs_toplevel.cpp     # Public API entry points (with thread safety)
│   ├── example/               # C++ VFS interface, backends, and FUSE
│   │   ├── lfs_interface.h      # IFileSystemDevice, IFileObject, FileHandle
│   │   ├── lfs_interface.cpp    # Thread-safe VFS implementation and block device bridge
│   │   ├── lfs_fuse.cpp         # Linux FUSE 3 driver
│   │   ├── file_backend.h       # File backend adapter
│   │   ├── memory_backend.h     # Memory backend adapter
│   │   └── example.cpp          # Demo application
│   └── tests/                 # Verification & Chaos Test Suite
│       └── test_main.cpp        # Tests: CRUD, Auto-Grow, Concurrency, Crypto, Fault-Injection
└── docs/                      # Documentation
    ├── api_reference.md       # Complete C++ API & types reference
    ├── architecture_deep_dive.md # CTZ skip-lists, dual-block pairs & TLV logs
    ├── benchmarks.md          # Comprehensive throughput and latency metrics
    └── vfs_value_and_architecture.md # Commercial value, ROI, and use-cases
```

---

## 📚 Documentation Index

* 📖 [**C++ API Reference**](docs/api_reference.md) — Detailed class and function documentation for `IFileSystemDevice`, `IFileObject`, `FileHandle`, `IBlockDevice`, and `Result<T, E>`.
* 🔬 [**Architecture Deep Dive**](docs/architecture_deep_dive.md) — In-depth explanation of CTZ skip-lists, dual-block metadata pairs, TLV commit logs, and lookahead allocation.
* ⚡ [**Performance Benchmarks**](docs/benchmarks.md) — Comprehensive throughput (up to 16.8 GB/s), IOPS (> 834k), latency, and multi-threading scalability reports.
* 💼 [**Commercial Value & Use Cases**](docs/vfs_value_and_architecture.md) — Business ROI, cloud IOPS cost reduction, IP protection/encryption, and plugin isolation.

