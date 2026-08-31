#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <thread>
#include <numeric>
#include <cstring>
#include <cmath>

#include "lfs_interface.h"
#include "lfs_block_device.h"

using Clock = std::chrono::high_resolution_clock;

struct BenchResult {
    std::string test_name;
    double data_mb;
    double elapsed_ms;
    double throughput_mbs;
    double iops;
    double avg_latency_us;
};

static void print_result_row(const BenchResult& r) {
    std::cout << "| " << std::left << std::setw(38) << r.test_name
              << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(1) << r.data_mb << " MB"
              << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(2) << r.elapsed_ms << " ms"
              << " | " << std::right << std::setw(9) << std::fixed << std::setprecision(2) << r.throughput_mbs << " MB/s"
              << " | " << std::right << std::setw(10) << std::fixed << std::setprecision(0) << r.iops
              << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(2) << r.avg_latency_us << " us"
              << " |" << std::endl;
}

static void print_header(const std::string& title) {
    std::cout << "\n### " << title << "\n" << std::endl;
    std::cout << "| Operation                              | Data Size | Time (ms) | Throughput |   IOPS     | Avg Latency |" << std::endl;
    std::cout << "|:---------------------------------------|----------:|----------:|-----------:|-----------:|------------:|" << std::endl;
}

// Benchmark in Memory Backend
static void benchmark_memory_backend() {
    print_header("1. RAM In-Memory Virtual File System (MemoryBlockDevice)");
    auto dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 512); // 32 MB
    auto vfs = fs::createVFSWithDevice(dev).value();

    constexpr size_t TOTAL_BYTES = 20 * 1024 * 1024; // 20 MB
    constexpr size_t CHUNK_SIZE = 64 * 1024;         // 64 KB
    std::vector<uint8_t> chunk(CHUNK_SIZE, 0x5A);

    // Sequential Write (64 KB chunks)
    {
        auto f = vfs->open("seq_64k.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->write(chunk.data(), CHUNK_SIZE);
        }
        handle->flush();
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Sequential Write (64 KB blocks)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }

    // Sequential Read (64 KB chunks)
    {
        auto f = vfs->open("seq_64k.bin", fs::kFileRead).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;
        std::vector<uint8_t> read_buf(CHUNK_SIZE);

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->read(read_buf.data(), CHUNK_SIZE);
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Sequential Read (64 KB blocks)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }

    // Random Read (4 KB blocks via Page Cache)
    {
        auto f = vfs->open("seq_64k.bin", fs::kFileRead).value();
        fs::FileHandle handle(f);
        constexpr size_t OPS = 50000;
        constexpr size_t BLOCK_4K = 4096;
        std::vector<uint8_t> buf(BLOCK_4K);

        std::mt19937 rng(1337);
        std::uniform_int_distribution<uint64_t> dist(0, TOTAL_BYTES - BLOCK_4K);

        std::vector<uint64_t> offsets(OPS);
        for (size_t i = 0; i < OPS; ++i) offsets[i] = dist(rng);

        auto t0 = Clock::now();
        for (size_t i = 0; i < OPS; ++i) {
            handle->seek(offsets[i], fs::IFileObject::kSeekSet);
            handle->read(buf.data(), BLOCK_4K);
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = (OPS * BLOCK_4K) / (1024.0 * 1024.0);
        BenchResult r{"Random Read (4 KB Page Cache)", mb, ms, (mb / (ms / 1000.0)), OPS / (ms / 1000.0), (ms * 1000.0) / OPS};
        print_result_row(r);
    }

    // Random Overwrite (4 KB blocks + Batched Flush)
    {
        auto f = vfs->open("seq_64k.bin", fs::kFileRead | fs::kFileWrite).value();
        fs::FileHandle handle(f);
        constexpr size_t OPS = 20000;
        constexpr size_t BLOCK_4K = 4096;
        std::vector<uint8_t> write_data(BLOCK_4K, 0xCC);

        std::mt19937 rng(2026);
        std::uniform_int_distribution<uint64_t> dist(0, TOTAL_BYTES - BLOCK_4K);
        std::vector<uint64_t> offsets(OPS);
        for (size_t i = 0; i < OPS; ++i) offsets[i] = dist(rng);

        auto t0 = Clock::now();
        for (size_t i = 0; i < OPS; ++i) {
            handle->seek(offsets[i], fs::IFileObject::kSeekSet);
            handle->write(write_data.data(), BLOCK_4K);
        }
        handle->flush();
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = (OPS * BLOCK_4K) / (1024.0 * 1024.0);
        BenchResult r{"Random Overwrite (4 KB + Flush)", mb, ms, (mb / (ms / 1000.0)), OPS / (ms / 1000.0), (ms * 1000.0) / OPS};
        print_result_row(r);
    }
}

// Benchmark File Container Backend
static void benchmark_file_backend() {
    print_header("2. Host File Container VFS (FileBlockDevice on NVMe/SSD)");
    std::string test_file = "bench_container.vfs";
    std::remove(test_file.c_str());

    auto dev = std::make_shared<fs::FileBlockDevice>(test_file, 64 * 1024, 320, true); // 20 MB
    auto vfs = fs::createVFSWithDevice(dev).value();

    constexpr size_t TOTAL_BYTES = 16 * 1024 * 1024; // 16 MB
    constexpr size_t CHUNK_SIZE = 64 * 1024;         // 64 KB
    std::vector<uint8_t> chunk(CHUNK_SIZE, 0x33);

    // Sequential Write
    {
        auto f = vfs->open("host_data.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->write(chunk.data(), CHUNK_SIZE);
        }
        handle->flush();
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Sequential Write to Disk (64 KB)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }

    // Sequential Read
    {
        auto f = vfs->open("host_data.bin", fs::kFileRead).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;
        std::vector<uint8_t> read_buf(CHUNK_SIZE);

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->read(read_buf.data(), CHUNK_SIZE);
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Sequential Read from Disk (64 KB)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }

    // Random Read (4 KB)
    {
        auto f = vfs->open("host_data.bin", fs::kFileRead).value();
        fs::FileHandle handle(f);
        constexpr size_t OPS = 30000;
        constexpr size_t BLOCK_4K = 4096;
        std::vector<uint8_t> buf(BLOCK_4K);

        std::mt19937 rng(42);
        std::uniform_int_distribution<uint64_t> dist(0, TOTAL_BYTES - BLOCK_4K);
        std::vector<uint64_t> offsets(OPS);
        for (size_t i = 0; i < OPS; ++i) offsets[i] = dist(rng);

        auto t0 = Clock::now();
        for (size_t i = 0; i < OPS; ++i) {
            handle->seek(offsets[i], fs::IFileObject::kSeekSet);
            handle->read(buf.data(), BLOCK_4K);
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = (OPS * BLOCK_4K) / (1024.0 * 1024.0);
        BenchResult r{"Random Read (4 KB Page Cache)", mb, ms, (mb / (ms / 1000.0)), OPS / (ms / 1000.0), (ms * 1000.0) / OPS};
        print_result_row(r);
    }

    std::remove(test_file.c_str());
}

// Benchmark Encrypted VFS
static void benchmark_crypto_backend() {
    print_header("3. Encrypted Block Transform Layer (CryptoBlockDevice)");
    auto raw_mem = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 320); // 20 MB
    auto crypto_dev = std::make_shared<fs::CryptoBlockDevice>(raw_mem, 0xCAFEBABEDEADBEEFULL);
    auto vfs = fs::createVFSWithDevice(crypto_dev).value();

    constexpr size_t TOTAL_BYTES = 16 * 1024 * 1024; // 16 MB
    constexpr size_t CHUNK_SIZE = 64 * 1024;         // 64 KB
    std::vector<uint8_t> chunk(CHUNK_SIZE, 0xEE);

    // Encrypted Write
    {
        auto f = vfs->open("crypto_vault.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->write(chunk.data(), CHUNK_SIZE);
        }
        handle->flush();
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Encrypted Seq Write (64 KB)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }

    // Encrypted Read
    {
        auto f = vfs->open("crypto_vault.bin", fs::kFileRead).value();
        fs::FileHandle handle(f);
        size_t ops = TOTAL_BYTES / CHUNK_SIZE;
        std::vector<uint8_t> read_buf(CHUNK_SIZE);

        auto t0 = Clock::now();
        for (size_t i = 0; i < ops; ++i) {
            handle->read(read_buf.data(), CHUNK_SIZE);
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = TOTAL_BYTES / (1024.0 * 1024.0);
        BenchResult r{"Encrypted Seq Read (64 KB)", mb, ms, (mb / (ms / 1000.0)), ops / (ms / 1000.0), (ms * 1000.0) / ops};
        print_result_row(r);
    }
}

// Benchmark Multithreaded Scalability
static void benchmark_multithreaded() {
    print_header("4. Multithreaded Concurrent Throughput (Thread-Safety Scalability)");
    auto dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 512); // 32 MB
    auto vfs = fs::createVFSWithDevice(dev).value();

    for (int num_threads : {2, 4, 8}) {
        const size_t bytes_per_thread = 2 * 1024 * 1024; // 2 MB per thread
        const size_t total_bytes = bytes_per_thread * num_threads;
        const size_t chunk_size = 32 * 1024;
        const size_t ops_per_thread = bytes_per_thread / chunk_size;

        std::vector<std::thread> workers;
        auto t0 = Clock::now();

        for (int t = 0; t < num_threads; ++t) {
            workers.emplace_back([vfs, t, ops_per_thread, chunk_size]() {
                std::string fname = "thread_" + std::to_string(t) + ".dat";
                auto f = vfs->open(fname, fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists).value();
                fs::FileHandle handle(f);
                std::vector<uint8_t> buf(chunk_size, static_cast<uint8_t>(t));

                for (size_t op = 0; op < ops_per_thread; ++op) {
                    handle->write(buf.data(), chunk_size);
                }
                handle->flush();
            });
        }

        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        auto t1 = Clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double mb = total_bytes / (1024.0 * 1024.0);
        size_t total_ops = ops_per_thread * num_threads;
        std::string label = "Concurrent Write (" + std::to_string(num_threads) + " threads)";
        BenchResult r{label, mb, ms, (mb / (ms / 1000.0)), total_ops / (ms / 1000.0), (ms * 1000.0) / total_ops};
        print_result_row(r);
    }
}

int main() {
    std::cout << "==========================================================================================" << std::endl;
    std::cout << "               littlefs_v2 High-Performance VFS Throughput & IOPS Benchmark               " << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    benchmark_memory_backend();
    benchmark_file_backend();
    benchmark_crypto_backend();
    benchmark_multithreaded();

    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << "Benchmark completed successfully." << std::endl;
    return 0;
}
