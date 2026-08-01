#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cassert>

#include "lfs_interface.h"

int main() {
    std::cout << "=== littlefs_v2 High-Performance VFS Benchmark ===" << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> filesystem;
    if (fs::createVFS(L"test_perf.fs", filesystem, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        std::cerr << "Failed to create VFS" << std::endl;
        return 1;
    }

    std::shared_ptr<fs::IFileObject> file;
    if (filesystem->openFile(file, "perf_test.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists) != fs::kCodeOK) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    constexpr size_t FILE_SIZE = 512 * 1024; // 512 KB
    std::vector<uint8_t> initial_data(FILE_SIZE);
    for (size_t i = 0; i < FILE_SIZE; ++i) {
        initial_data[i] = static_cast<uint8_t>(i & 0xFF);
    }

    // 1. Sequential write 512 KB
    auto start = std::chrono::high_resolution_clock::now();
    file->write(initial_data.data(), FILE_SIZE);
    file->flush();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "[1] Sequential write (512 KB): "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // 2. Random Seek & Read (5,000 ops)
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist_offset(0, FILE_SIZE - 64);
    constexpr size_t READ_OPS = 5000;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < READ_OPS; ++i) {
        size_t off = dist_offset(rng);
        file->seek(off, fs::IFileObject::kSeekSet);
        uint8_t buf[64];
        file->read(buf, sizeof(buf));
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "[2] Random Seek & Read (5,000 ops): "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // 3. Random Writes (1,000 ops across 512 KB)
    constexpr size_t WRITE_OPS = 1000;
    std::vector<std::pair<size_t, uint8_t>> expected_changes;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < WRITE_OPS; ++i) {
        size_t off = dist_offset(rng);
        uint8_t val = static_cast<uint8_t>(i ^ 0xAA);
        file->seek(off, fs::IFileObject::kSeekSet);
        file->write(&val, 1);
        expected_changes.push_back({off, val});
    }
    file->flush();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "[3] Random Overwrite (1,000 ops + Batched Flush): "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // 4. Verification
    file->seek(0, fs::IFileObject::kSeekSet);
    std::vector<uint8_t> read_back(FILE_SIZE);
    file->read(read_back.data(), FILE_SIZE);

    for (const auto& change : expected_changes) {
        initial_data[change.first] = change.second;
    }

    bool verify_ok = (initial_data == read_back);
    std::cout << "[4] Data Integrity Verification: " << (verify_ok ? "PASSED (100% OK)" : "FAILED") << std::endl;

    return verify_ok ? 0 : 1;
}
