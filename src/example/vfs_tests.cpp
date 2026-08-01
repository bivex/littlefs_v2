#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <random>
#include "lfs_interface.h"

// Simple Lightweight Test Framework
static int g_test_passed = 0;
static int g_test_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            g_test_passed++; \
        } else { \
            g_test_failed++; \
            std::cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl; \
        } \
    } while (0)

void test_basic_file_io(fs::lfsVFS::Backend backend, const std::wstring& fs_name) {
    std::cout << "[Test Suite] Basic File I/O (Backend: " 
              << (backend == fs::lfsVFS::Backend::kMemoryBackend ? "Memory" : "File") << ")..." << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs;
    TEST_ASSERT(fs::createVFS(fs_name, vfs, backend) == fs::kCodeOK, "VFS creation");

    std::shared_ptr<fs::IFileObject> file;
    TEST_ASSERT(vfs->openFile(file, "hello.txt", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK, "File open");

    std::string text = "Hello littlefs_v2 high performance VFS!";
    TEST_ASSERT(file->write(text.c_str(), text.size()) == static_cast<int64_t>(text.size()), "File write");
    file->flush();

    TEST_ASSERT(file->size() == static_cast<int64_t>(text.size()), "File size check");
    TEST_ASSERT(file->tell() == static_cast<int64_t>(text.size()), "File tell check");

    TEST_ASSERT(file->seek(0, fs::IFileObject::kSeekSet) == 0, "Seek to start");

    std::vector<char> read_buf(text.size() + 1, 0);
    TEST_ASSERT(file->read(read_buf.data(), text.size()) == static_cast<int64_t>(text.size()), "File read");
    TEST_ASSERT(std::string(read_buf.data()) == text, "Content verification");

    file.reset();
    TEST_ASSERT(vfs->existsFile("hello.txt") == fs::kCodeOK, "Exists file check");
    TEST_ASSERT(vfs->deleteFile("hello.txt") == fs::kCodeOK, "Delete file check");
    TEST_ASSERT(vfs->existsFile("hello.txt") == fs::kCodeFileNotFound, "File deleted check");
}

void test_random_access_and_flush(fs::lfsVFS::Backend backend, const std::wstring& fs_name) {
    std::cout << "[Test Suite] Random Access Overwrite & Page Cache Flush..." << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs;
    TEST_ASSERT(fs::createVFS(fs_name, vfs, backend) == fs::kCodeOK, "VFS creation");

    std::shared_ptr<fs::IFileObject> file;
    TEST_ASSERT(vfs->openFile(file, "random_io.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK, "File open");

    constexpr size_t FILE_SIZE = 128 * 1024; // 128 KB
    std::vector<uint8_t> shadow_data(FILE_SIZE, 0x00);
    file->write(shadow_data.data(), FILE_SIZE);
    file->flush();

    std::mt19937 rng(777);
    std::uniform_int_distribution<size_t> off_dist(0, FILE_SIZE - 32);

    constexpr size_t NUM_WRITES = 500;
    for (size_t i = 0; i < NUM_WRITES; ++i) {
        size_t off = off_dist(rng);
        uint8_t val = static_cast<uint8_t>(i & 0xFF);
        file->seek(off, fs::IFileObject::kSeekSet);
        file->write(&val, 1);
        shadow_data[off] = val;
    }

    file->flush(); // Batched flush

    file->seek(0, fs::IFileObject::kSeekSet);
    std::vector<uint8_t> read_back(FILE_SIZE);
    file->read(read_back.data(), FILE_SIZE);

    TEST_ASSERT(shadow_data == read_back, "Random overwrite data integrity after flush");

    vfs->deleteFile("random_io.bin");
}

void test_seek_modes(fs::lfsVFS::Backend backend, const std::wstring& fs_name) {
    std::cout << "[Test Suite] Seek Modes (SET, CUR, END)..." << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs;
    fs::createVFS(fs_name, vfs, backend);

    std::shared_ptr<fs::IFileObject> file;
    vfs->openFile(file, "seek_test.txt", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);

    std::string pattern = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    file->write(pattern.c_str(), pattern.size());
    file->flush();

    TEST_ASSERT(file->seek(10, fs::IFileObject::kSeekSet) == 10, "Seek SET to 10");
    char c = 0;
    file->read(&c, 1);
    TEST_ASSERT(c == 'A', "Read char at offset 10");

    TEST_ASSERT(file->seek(5, fs::IFileObject::kSeekCur) == 16, "Seek CUR +5 from 11");
    file->read(&c, 1);
    TEST_ASSERT(c == 'G', "Read char at offset 16");

    TEST_ASSERT(file->seek(-1, fs::IFileObject::kSeekEnd) == static_cast<int64_t>(pattern.size() - 1), "Seek END -1");
    file->read(&c, 1);
    TEST_ASSERT(c == 'Z', "Read char at last offset");

    vfs->deleteFile("seek_test.txt");
}

void test_truncate(fs::lfsVFS::Backend backend, const std::wstring& fs_name) {
    std::cout << "[Test Suite] File Truncate..." << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs;
    fs::createVFS(fs_name, vfs, backend);

    std::shared_ptr<fs::IFileObject> file;
    vfs->openFile(file, "trunc.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);

    std::vector<uint8_t> data(1000, 0xFF);
    file->write(data.data(), data.size());
    file->flush();

    TEST_ASSERT(file->size() == 1000, "Size before truncate");
    TEST_ASSERT(file->truncate(250) == fs::kCodeOK, "Truncate to 250 bytes");
    TEST_ASSERT(file->size() == 250, "Size after truncate");

    vfs->deleteFile("trunc.bin");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "          littlefs_v2 Unit Test Suite Execution           " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Run tests on File Backend
    test_basic_file_io(fs::lfsVFS::Backend::kFileBackend, L"test_file_vfs.fs");
    test_random_access_and_flush(fs::lfsVFS::Backend::kFileBackend, L"test_file_vfs.fs");
    test_seek_modes(fs::lfsVFS::Backend::kFileBackend, L"test_file_vfs.fs");
    test_truncate(fs::lfsVFS::Backend::kFileBackend, L"test_file_vfs.fs");

    // Run tests on Memory Backend
    test_basic_file_io(fs::lfsVFS::Backend::kMemoryBackend, L"test_mem_vfs.fs");
    test_random_access_and_flush(fs::lfsVFS::Backend::kMemoryBackend, L"test_mem_vfs.fs");
    test_seek_modes(fs::lfsVFS::Backend::kMemoryBackend, L"test_mem_vfs.fs");
    test_truncate(fs::lfsVFS::Backend::kMemoryBackend, L"test_mem_vfs.fs");

    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << "SUMMARY: Passed: " << g_test_passed << " | Failed: " << g_test_failed << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    return g_test_failed == 0 ? 0 : 1;
}
