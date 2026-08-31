#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <memory>
#include "lfs_interface.h"

static bool seed_committed_files(const std::string& fs_path, int count) {
    std::shared_ptr<fs::IFileSystemDevice> vfs;
    if (fs::createVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        return false;
    }

    for (int i = 0; i < count; ++i) {
        std::string path = "committed_state_" + std::to_string(i) + ".bin";
        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) != fs::kCodeOK) {
            return false;
        }

        std::vector<uint8_t> payload(512);
        for (size_t b = 0; b < payload.size(); ++b) {
            payload[b] = static_cast<uint8_t>((i + b) & 0xFF);
        }
        file->write(payload.data(), payload.size());
        file->flush();
    }
    return true;
}

static void inject_unflushed_crash_writes(const std::string& fs_path, int start_idx, int count) {
    std::shared_ptr<fs::IFileSystemDevice> vfs;
    if (fs::openVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        return;
    }

    for (int i = start_idx; i < start_idx + count; ++i) {
        std::string path = "unflushed_state_" + std::to_string(i) + ".bin";
        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
            std::string junk = "Unflushed transient data in RAM page cache";
            file->write(junk.data(), junk.size());
            // Intentionally omit flush() to simulate sudden power cutoff
        }
    }
}

static bool verify_single_file(std::shared_ptr<fs::IFileSystemDevice> vfs, int file_idx) {
    std::string path = "committed_state_" + std::to_string(file_idx) + ".bin";
    std::shared_ptr<fs::IFileObject> file;
    if (vfs->openFile(file, path, fs::kFileRead) != fs::kCodeOK) {
        return false;
    }

    std::vector<uint8_t> payload(file->size());
    file->read(payload.data(), payload.size());
    if (payload.size() != 512) return false;

    for (size_t b = 0; b < payload.size(); ++b) {
        if (payload[b] != static_cast<uint8_t>((file_idx + b) & 0xFF)) {
            return false;
        }
    }
    return true;
}

static bool verify_integrity_after_crash(const std::string& fs_path, int expected_count) {
    std::shared_ptr<fs::IFileSystemDevice> vfs;
    if (fs::openVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        return false;
    }

    int intact_files = 0;
    for (int i = 0; i < expected_count; ++i) {
        if (verify_single_file(vfs, i)) {
            intact_files++;
        }
    }

    std::cout << " -> Committed files intact: " << intact_files << " / " << expected_count << std::endl;
    auto entries = vfs->dir("/");
    std::cout << " -> Total valid entries in VFS root dir after crash recovery: " << entries.size() << std::endl;

    return (intact_files == expected_count);
}

void run_power_loss_fault_injection_test() {
    std::cout << "==================================================================" << std::endl;
    std::cout << "  littlefs_v2 Power-Loss Fault Injection & Crash Recovery Test   " << std::endl;
    std::cout << "==================================================================" << std::endl;

    const std::string fs_path = "/tmp/power_loss_test.vfs";
    std::remove(fs_path.c_str());

    std::cout << "\n[Phase 1] Writing & flushing 100 critical state files to disk..." << std::endl;
    bool phase1_ok = seed_committed_files(fs_path, 100);
    assert(phase1_ok);
    std::cout << " -> 100 files successfully committed and flushed." << std::endl;

    std::cout << "\n[Phase 2] Simulating active un-flushed writes & HARD POWER-LOSS CRASH..." << std::endl;
    inject_unflushed_crash_writes(fs_path, 100, 50);
    std::cout << " -> [CRASH INJECTED] Power disconnected mid-operation! No clean unmount." << std::endl;

    std::cout << "\n[Phase 3] Remounting VFS after power loss crash & verifying integrity..." << std::endl;
    bool passed = verify_integrity_after_crash(fs_path, 100);

    if (passed) {
        std::cout << "\n>>> POWER-LOSS TEST VERDICT: 100% PASSED! <<<" << std::endl;
    } else {
        std::cerr << "\n>>> POWER-LOSS TEST VERDICT: FAILED! <<<" << std::endl;
    }
}
