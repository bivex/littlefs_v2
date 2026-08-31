# littlefs_v2 Performance Benchmarks

Detailed performance benchmarks for `littlefs_v2` measuring throughput, latency, IOPS, and scalability across RAM, NVMe host storage, and cryptographic transform layers.

---

## Benchmark Environment
* **Platform**: Apple Silicon / ARM64
* **Compiler**: Apple Clang 17.0.0 (`-std=c++20 -O3 -DNDEBUG`)
* **Build System**: CMake 4.3 + Ninja
* **Page Cache**: 4096 bytes per page
* **Block Size**: 64 KB (65,536 bytes)

---

## 1. Summary Results Table

| Storage Backend / Scenario | Operation | Block Size | Throughput | IOPS | Avg Latency |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **RAM (`MemoryBlockDevice`)** | Sequential Write | 64 KB | **`3 680 MB/s`** | 58 880 | 16.98 µs |
| **RAM (`MemoryBlockDevice`)** | Sequential Read | 64 KB | **`16 810 MB/s`** | 268 973 | 3.72 µs |
| **RAM (`MemoryBlockDevice`)** | Random Read (Cache) | 4 KB | **`3 258 MB/s`** | **`834 238`** | **`1.20 µs`** |
| **RAM (`MemoryBlockDevice`)** | Random Overwrite + Flush | 4 KB | **`104.8 MB/s`** | 26 818 | 37.29 µs |
| **NVMe (`FileBlockDevice`)** | Sequential Write to Disk | 64 KB | **`2 675 MB/s`** | 42 808 | 23.36 µs |
| **NVMe (`FileBlockDevice`)** | Sequential Read from Disk | 64 KB | **`6 211 MB/s`** | 99 387 | 10.06 µs |
| **NVMe (`FileBlockDevice`)** | Random Read (Cache) | 4 KB | **`747 MB/s`** | **`191 250`** | 5.23 µs |
| **Crypto (`CryptoBlockDevice`)**| Encrypted Seq Write | 64 KB | **`1 353 MB/s`** | 21 650 | 46.19 µs |
| **Crypto (`CryptoBlockDevice`)**| Encrypted Seq Read | 64 KB | **`3 250 MB/s`** | 52 012 | 19.23 µs |

---

## 2. Multithreaded Concurrency Scaling

Measured under heavy concurrent write operations with full recursive mutex protection:

```text
2 Threads: [██████████████████████████████] 4,496 MB/s (143,888 IOPS)
4 Threads: [███████████████               ] 2,273 MB/s ( 72,753 IOPS)
8 Threads: [█████████████████████         ] 3,192 MB/s (102,172 IOPS)
```

---

## 3. Running Benchmarks Locally

To reproduce the benchmarks on your machine:

```bash
# Build benchmark runner
cmake --preset default
cmake --build --preset default

# Execute
./build/benchmark_throughput
```
