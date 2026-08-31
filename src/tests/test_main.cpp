#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <cassert>
#include <cstring>
#include <sstream>
#include "lfs_interface.h"
#include "lfs_block_device.h"

#define TEST_CHECK(condition, msg) \
    do { \
        if (!(condition)) { \
            std::cerr << "[-] Assertion failed: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

// 1. Basic CRUD and Directory test
static bool test_basic_crud() {
    std::cout << "[*] Running Test: Basic CRUD & Directory..." << std::endl;
    auto mem_dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 8);
    auto vfs_res = fs::createVFSWithDevice(mem_dev);
    TEST_CHECK(vfs_res.has_value(), "Failed to create in-memory VFS");
    auto vfs = vfs_res.value();

    // Create & write to a file
    auto file_res = vfs->open("test.txt", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
    TEST_CHECK(file_res.has_value(), "Failed to open test.txt for writing");
    fs::FileHandle file(file_res.value());

    const std::string test_data = "Hello LittleFS v2 with thread safety and modern RAII!";
    int64_t written = file->write(test_data.c_str(), test_data.size());
    TEST_CHECK(written == static_cast<int64_t>(test_data.size()), "Write size mismatch");
    file->flush();

    // Verify file size
    TEST_CHECK(file->size() == static_cast<int64_t>(test_data.size()), "File size mismatch");

    // Seek and read
    file->seek(0, fs::IFileObject::kSeekSet);
    std::vector<char> read_buf(test_data.size() + 1, 0);
    int64_t bytes_read = file->read(read_buf.data(), test_data.size());
    TEST_CHECK(bytes_read == static_cast<int64_t>(test_data.size()), "Read size mismatch");
    TEST_CHECK(std::string(read_buf.data()) == test_data, "Read content mismatch");

    // Test directory listing
    auto entries_res = vfs->listDir("/");
    TEST_CHECK(entries_res.has_value(), "Failed to list directory");
    auto entries = entries_res.value();
    TEST_CHECK(entries.size() == 1, "Expected 1 directory entry");
    TEST_CHECK(entries[0].getPath() == "test.txt", "Entry name mismatch");
    TEST_CHECK(entries[0].getSize() == test_data.size(), "Entry size mismatch");

    // Rename file
    auto rename_res = vfs->rename("test.txt", "renamed.txt");
    TEST_CHECK(rename_res.has_value(), "Rename failed");
    TEST_CHECK(vfs->existsFile("test.txt") == fs::kCodeFileNotFound, "Old file still exists");
    TEST_CHECK(vfs->existsFile("renamed.txt") == fs::kCodeOK, "Renamed file not found");

    // Delete file
    auto del_res = vfs->remove("renamed.txt");
    TEST_CHECK(del_res.has_value(), "Delete file failed");
    TEST_CHECK(vfs->existsFile("renamed.txt") == fs::kCodeFileNotFound, "Deleted file still exists");

    std::cout << "[+] Test Basic CRUD passed successfully!" << std::endl;
    return true;
}

// 2. Auto-Grow Test
static bool test_autogrow() {
    std::cout << "[*] Running Test: Storage Auto-Grow..." << std::endl;
    // Initial 4 blocks = 256KB
    auto mem_dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 4);
    auto vfs_res = fs::createVFSWithDevice(mem_dev);
    TEST_CHECK(vfs_res.has_value(), "Failed to create VFS");
    auto vfs = vfs_res.value();

    // Write multiple files that exceed the initial 4 blocks (256 KB)
    const size_t chunk_size = 32 * 1024;
    std::vector<uint8_t> payload(chunk_size, 0xAB);

    for (int i = 0; i < 15; ++i) {
        std::string filename = "file_" + std::to_string(i) + ".dat";
        auto f_res = vfs->open(filename, fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
        TEST_CHECK(f_res.has_value(), "Failed to open file during auto-grow loop");
        fs::FileHandle f(f_res.value());

        int64_t w = f->write(payload.data(), payload.size());
        TEST_CHECK(w == static_cast<int64_t>(payload.size()), "Write failed during auto-grow");
        f->flush();
    }

    // Verify all 15 files are intact
    for (int i = 0; i < 15; ++i) {
        std::string filename = "file_" + std::to_string(i) + ".dat";
        auto f_res = vfs->open(filename, fs::kFileRead);
        TEST_CHECK(f_res.has_value(), "Failed to open file for read verification");
        fs::FileHandle f(f_res.value());

        std::vector<uint8_t> readback(chunk_size, 0);
        int64_t r = f->read(readback.data(), chunk_size);
        TEST_CHECK(r == static_cast<int64_t>(chunk_size), "Readback size mismatch");
        TEST_CHECK(std::memcmp(readback.data(), payload.data(), chunk_size) == 0, "Data corruption detected");
    }

    TEST_CHECK(mem_dev->get_block_count() > 4, "Block device was not auto-grown");
    std::cout << "[+] Test Auto-Grow passed! Blocks grew from 4 to " << mem_dev->get_block_count() << std::endl;
    return true;
}

// 3. Multi-Threaded Stress Test (Thread Safety)
static bool test_thread_safety() {
    std::cout << "[*] Running Test: Multi-Threaded Concurrency..." << std::endl;
    auto mem_dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 16);
    auto vfs_res = fs::createVFSWithDevice(mem_dev);
    TEST_CHECK(vfs_res.has_value(), "Failed to create VFS");
    auto vfs = vfs_res.value();

    const int num_threads = 8;
    const int ops_per_thread = 20;
    std::vector<std::thread> workers;
    std::atomic<bool> all_ok{true};

    for (int t = 0; t < num_threads; ++t) {
        workers.emplace_back([vfs, t, ops_per_thread, &all_ok]() {
            for (int op = 0; op < ops_per_thread; ++op) {
                std::string fname = "th_" + std::to_string(t) + "_f_" + std::to_string(op) + ".txt";
                std::string data = "Thread " + std::to_string(t) + " Data iteration " + std::to_string(op);

                auto open_res = vfs->open(fname, fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
                if (!open_res.has_value()) {
                    all_ok = false;
                    return;
                }

                fs::FileHandle handle(open_res.value());
                handle->write(data.c_str(), data.size());
                handle->flush();

                handle->seek(0, fs::IFileObject::kSeekSet);
                std::vector<char> buf(data.size() + 1, 0);
                int64_t r = handle->read(buf.data(), data.size());
                if (r != static_cast<int64_t>(data.size()) || std::string(buf.data()) != data) {
                    all_ok = false;
                    return;
                }

                // Randomly perform directory listing
                if (op % 5 == 0) {
                    auto entries = vfs->listDir("/");
                    if (!entries.has_value()) {
                        all_ok = false;
                        return;
                    }
                }
            }
        });
    }

    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }

    TEST_CHECK(all_ok.load(), "Concurrent operations failed");
    std::cout << "[+] Test Multi-Threaded Concurrency passed with " << num_threads << " active threads!" << std::endl;
    return true;
}

// 4. Crypto / Block Transform Layer Test
static bool test_crypto_layer() {
    std::cout << "[*] Running Test: Pluggable Crypto / Block Transform Layer..." << std::endl;
    auto raw_mem = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 8);
    auto crypto_dev = std::make_shared<fs::CryptoBlockDevice>(raw_mem, 0xFEEDFACECAFEBEEFULL);

    auto vfs_res = fs::createVFSWithDevice(crypto_dev);
    TEST_CHECK(vfs_res.has_value(), "Failed to create encrypted VFS");
    auto vfs = vfs_res.value();

    const std::string secret_text = "Highly Confidential Secret Message inside LittleFS V2!";
    auto f_res = vfs->open("secret.txt", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
    TEST_CHECK(f_res.has_value(), "Failed to open secret.txt");
    fs::FileHandle f(f_res.value());
    f->write(secret_text.c_str(), secret_text.size());
    f->flush();

    // Verify reading back through the crypto device works
    f->seek(0, fs::IFileObject::kSeekSet);
    std::vector<char> buf(secret_text.size() + 1, 0);
    f->read(buf.data(), secret_text.size());
    TEST_CHECK(std::string(buf.data()) == secret_text, "Decrypted text does not match original plaintext");

    // Inspect underlying raw memory storage to verify plaintext is NOT present anywhere on raw disk
    const auto& raw_storage = raw_mem->get_storage();
    std::string raw_str(reinterpret_cast<const char*>(raw_storage.data()), raw_storage.size());
    TEST_CHECK(raw_str.find(secret_text) == std::string::npos, "Plaintext found in raw block storage! Encryption failed.");

    std::cout << "[+] Test Crypto Layer passed! Raw storage contains encrypted cipher data." << std::endl;
    return true;
}

// 5. Fault Injection & Power-Loss Recovery Test
static bool test_fault_injection() {
    std::cout << "[*] Running Test: Fault Injection & Power-Loss Recovery..." << std::endl;
    auto raw_mem = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 8);
    auto fault_dev = std::make_shared<fs::FaultInjectBlockDevice>(raw_mem);

    // 1. Create and populate initial valid state
    {
        auto vfs_res = fs::createVFSWithDevice(fault_dev);
        TEST_CHECK(vfs_res.has_value(), "Failed to create VFS");
        auto vfs = vfs_res.value();

        auto f_res = vfs->open("important.doc", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
        TEST_CHECK(f_res.has_value(), "Failed to open important.doc");
        fs::FileHandle f(f_res.value());
        const std::string content = "Version 1.0 - Committed successfully";
        f->write(content.c_str(), content.size());
        f->flush();
    }

    // 2. Simulate write failure during an update (e.g. power-loss during write)
    {
        auto vfs_res = fs::openVFSWithDevice(fault_dev);
        TEST_CHECK(vfs_res.has_value(), "Failed to remount VFS");
        auto vfs = vfs_res.value();

        // Inject sudden failure after 1 write operation
        fault_dev->set_fail_after_writes(1);

        auto f_res = vfs->open("incomplete.tmp", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
        if (f_res.has_value()) {
            fs::FileHandle f(f_res.value());
            std::vector<uint8_t> large_data(128 * 1024, 0xEE);
            // This write or sync will fail due to injected power cut
            f->write(large_data.data(), large_data.size());
            f->flush();
        }
    }

    // 3. Clear fault and remount: LittleFS must cleanly recover to the previous consistent state
    fault_dev->set_fail_after_writes(-1);
    {
        auto vfs_res = fs::openVFSWithDevice(fault_dev);
        TEST_CHECK(vfs_res.has_value(), "Filesystem failed to mount after power loss recovery!");
        auto vfs = vfs_res.value();

        // Verify the original important.doc is completely intact
        auto f_res = vfs->open("important.doc", fs::kFileRead);
        TEST_CHECK(f_res.has_value(), "Original file lost after recovery");
        fs::FileHandle f(f_res.value());

        const std::string expected = "Version 1.0 - Committed successfully";
        std::vector<char> buf(expected.size() + 1, 0);
        f->read(buf.data(), expected.size());
        TEST_CHECK(std::string(buf.data()) == expected, "Recovered file content corrupted");
    }

    std::cout << "[+] Test Fault Injection & Power-Loss Recovery passed! FS cleanly recovered." << std::endl;
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " LittleFS v2 Test Suite & Verification " << std::endl;
    std::cout << "========================================" << std::endl;

    bool ok = true;
    ok &= test_basic_crud();
    ok &= test_autogrow();
    ok &= test_thread_safety();
    ok &= test_crypto_layer();
    ok &= test_fault_injection();

    std::cout << "========================================" << std::endl;
    if (ok) {
        std::cout << ">>> ALL TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
        return 0;
    } else {
        std::cerr << ">>> SOME TESTS FAILED! <<<" << std::endl;
        return 1;
    }
}
