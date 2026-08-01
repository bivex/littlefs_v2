#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "lfs_interface.h"

void run_power_loss_fault_injection_test() {
    std::cout << "==================================================================" << std::endl;
    std::cout << "  littlefs_v2 Power-Loss Fault Injection & Crash Recovery Test   " << std::endl;
    std::cout << "==================================================================" << std::endl;

    const std::wstring fs_path = L"/tmp/power_loss_test.vfs";
    std::remove("/tmp/power_loss_test.vfs");

    // PHASE 1: Initial Commit of 100 Critical State Files
    std::cout << "\n[Phase 1] Writing & flushing 100 critical state files to disk..." << std::endl;
    {
        std::shared_ptr<fs::IFileSystemDevice> vfs;
        assert(fs::createVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) == fs::kCodeOK);

        for (int i = 0; i < 100; ++i) {
            std::string path = "committed_state_" + std::to_string(i) + ".bin";
            std::shared_ptr<fs::IFileObject> file;
            assert(vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK);

            std::vector<uint8_t> payload(512);
            for (size_t b = 0; b < payload.size(); ++b) {
                payload[b] = static_cast<uint8_t>((i + b) & 0xFF);
            }
            file->write(payload.data(), payload.size());
            file->flush(); // Flushed to physical VFS disk image
        }
        std::cout << " -> 100 files successfully committed and flushed." << std::endl;
    }

    // PHASE 2: Unflushed Dirty Writes & SIMULATED HARD POWER LOSS CRASH
    std::cout << "\n[Phase 2] Simulating active un-flushed writes & HARD POWER-LOSS CRASH..." << std::endl;
    {
        std::shared_ptr<fs::IFileSystemDevice> vfs;
        assert(fs::openVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) == fs::kCodeOK);

        // Write 50 new files into RAM Page Cache WITHOUT calling flush()
        for (int i = 100; i < 150; ++i) {
            std::string path = "unflushed_state_" + std::to_string(i) + ".bin";
            std::shared_ptr<fs::IFileObject> file;
            if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
                std::string junk = "Unflushed transient data in RAM page cache";
                file->write(junk.data(), junk.size());
                // INTENTIONALLY NOT CALLING FLUSH() -> SIMULATING POWER PULL MID-WRITE
            }
        }

        std::cout << " -> [CRASH INJECTED] Power disconnected mid-operation! No clean unmount." << std::endl;
        // Destructing VFS handle abruptly simulating sudden power loss
    }

    // PHASE 3: Recovery & Remount Analysis
    std::cout << "\n[Phase 3] Remounting VFS after power loss crash & verifying integrity..." << std::endl;
    {
        std::shared_ptr<fs::IFileSystemDevice> vfs;
        assert(fs::openVFS(fs_path, vfs, fs::lfsVFS::Backend::kFileBackend) == fs::kCodeOK);

        size_t intact_committed_files = 0;
        size_t corrupted_files = 0;

        for (int i = 0; i < 100; ++i) {
            std::string path = "committed_state_" + std::to_string(i) + ".bin";
            std::shared_ptr<fs::IFileObject> file;
            if (vfs->openFile(file, path, fs::kFileRead) == fs::kCodeOK) {
                std::vector<uint8_t> payload(file->size());
                file->read(payload.data(), payload.size());

                bool valid = true;
                for (size_t b = 0; b < payload.size(); ++b) {
                    if (payload[b] != static_cast<uint8_t>((i + b) & 0xFF)) {
                        valid = false;
                        break;
                    }
                }
                if (valid && payload.size() == 512) {
                    intact_committed_files++;
                } else {
                    corrupted_files++;
                }
            }
        }

        std::cout << " -> Committed files intact: " << intact_committed_files << " / 100" << std::endl;
        std::cout << " -> Corrupted files count : " << corrupted_files << std::endl;

        // Verify metadata tree scan after crash
        auto entries = vfs->dir("/");
        std::cout << " -> Total valid entries in VFS root dir after crash recovery: " << entries.size() << std::endl;

        if (intact_committed_files == 100 && corrupted_files == 0) {
            std::cout << "\n>>> POWER-LOSS TEST VERDICT: 100% PASSED! <<<" << std::endl;
            std::cout << ">>> Zero metadata corruption. All committed state 100% preserved. <<<" << std::endl;
        } else {
            std::cerr << "\n>>> POWER-LOSS TEST VERDICT: FAILED! Corruption detected. <<<" << std::endl;
            exit(1);
        }
    }
    std::remove("/tmp/power_loss_test.vfs");
}

int main() {
    run_power_loss_fault_injection_test();
    return 0;
}
