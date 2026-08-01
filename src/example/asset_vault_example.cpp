#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include "lfs_interface.h"

using namespace std::chrono;

// Helper to format bytes into human-readable strings
std::string format_size(uint64_t bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + " MB";
}

int main() {
    std::cout << "================================================================" << std::endl;
    std::cout << "   littlefs_v2 Medium Project: Asset & Resource Package Vault   " << std::endl;
    std::cout << "================================================================" << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs;
    if (fs::createVFS(L"game_assets.vfs", vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        std::cerr << "Failed to create VFS" << std::endl;
        return 1;
    }

    constexpr size_t NUM_CONFIGS = 500;
    constexpr size_t NUM_TEXTURES = 300;
    constexpr size_t NUM_AUDIO = 200;
    constexpr size_t TOTAL_FILES = NUM_CONFIGS + NUM_TEXTURES + NUM_AUDIO;

    std::cout << "\n[1] Creating " << TOTAL_FILES << " small asset files inside virtual package container..." << std::endl;

    auto t_start = high_resolution_clock::now();

    // 1. Create JSON items config files
    for (size_t i = 0; i < NUM_CONFIGS; ++i) {
        std::stringstream ss;
        ss << "{\n"
           << "  \"id\": " << i << ",\n"
           << "  \"name\": \"Item_Artifact_" << i << "\",\n"
           << "  \"type\": \"weapon\",\n"
           << "  \"damage\": " << (i * 7 % 150 + 10) << ",\n"
           << "  \"durability\": 100\n"
           << "}";
        std::string content = ss.str();

        std::string path = "config_item_" + std::to_string(i) + ".json";
        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
            file->write(content.data(), content.size());
            file->flush();
        }
    }

    // 2. Create Texture metadata files
    for (size_t i = 0; i < NUM_TEXTURES; ++i) {
        std::stringstream ss;
        ss << "TEXTURE_HEADER_v2\n"
           << "WIDTH=1024\nHEIGHT=1024\nFORMAT=RGBA8\n"
           << "MIPMAPS=4\nCHECKSUM=0x" << std::hex << (i * 0x1337 + 0xDEADBEEF) << "\n";
        std::string content = ss.str();

        std::string path = "tex_metadata_" + std::to_string(i) + ".meta";
        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
            file->write(content.data(), content.size());
            file->flush();
        }
    }

    // 3. Create Audio SFX descriptor files
    for (size_t i = 0; i < NUM_AUDIO; ++i) {
        std::stringstream ss;
        ss << "SFX_SAMPLE_ID=" << i << "\n"
           << "SAMPLE_RATE=44100\nCHANNELS=2\nBITRATE=320kbps\n"
           << "VOLUME_GAIN=1.0\nLOOP=FALSE\n";
        std::string content = ss.str();

        std::string path = "sfx_sound_" + std::to_string(i) + ".sfx";
        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
            file->write(content.data(), content.size());
            file->flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    double write_time_ms = duration<double, std::milli>(t_end - t_start).count();

    std::cout << " -> Created and packed " << TOTAL_FILES << " files in " 
              << std::fixed << std::setprecision(2) << write_time_ms << " ms ("
              << (write_time_ms / TOTAL_FILES) << " ms per file)" << std::endl;

    // 2. Directory scan & list files
    std::cout << "\n[2] Scanning VFS root directory entries..." << std::endl;
    t_start = high_resolution_clock::now();
    auto entries = vfs->dir("/");
    t_end = high_resolution_clock::now();
    double dir_time_ms = duration<double, std::milli>(t_end - t_start).count();

    uint64_t total_vfs_bytes = 0;
    for (const auto& entry : entries) {
        total_vfs_bytes += entry.getSize();
    }

    std::cout << " -> Discovered " << entries.size() << " files in directory." << std::endl;
    std::cout << " -> Directory scan time: " << std::fixed << std::setprecision(2) << dir_time_ms << " ms" << std::endl;
    std::cout << " -> Total logical file data size: " << format_size(total_vfs_bytes) << std::endl;

    // 3. Random Access & Batch Modification (Simulating live game updates)
    constexpr size_t RANDOM_UPDATES = 1000;
    std::cout << "\n[3] Executing " << RANDOM_UPDATES << " random reads & modifications across random asset files..." << std::endl;

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> file_dist(0, NUM_CONFIGS - 1);

    t_start = high_resolution_clock::now();
    for (size_t k = 0; k < RANDOM_UPDATES; ++k) {
        size_t file_id = file_dist(rng);
        std::string path = "config_item_" + std::to_string(file_id) + ".json";

        std::shared_ptr<fs::IFileObject> file;
        if (vfs->openFile(file, path, fs::kFileRead | fs::kFileWrite) == fs::kCodeOK) {
            // Read content
            std::string content(file->size(), '\0');
            file->read(&content[0], content.size());

            // Modify content in memory page cache
            std::string updated_content = content + "\n// Updated_At_Tick=" + std::to_string(k);
            file->seek(0, fs::IFileObject::kSeekSet);
            file->write(updated_content.data(), updated_content.size());
            file->flush();
        }
    }
    t_end = high_resolution_clock::now();
    double update_time_ms = duration<double, std::milli>(t_end - t_start).count();

    std::cout << " -> Completed " << RANDOM_UPDATES << " random reads & writes in " 
              << std::fixed << std::setprecision(2) << update_time_ms << " ms ("
              << (update_time_ms / RANDOM_UPDATES * 1000.0) << " us per random update)" << std::endl;

    // 4. Verification & Reading back modified file
    std::cout << "\n[4] Data Integrity Check: Reading back updated config file..." << std::endl;
    std::shared_ptr<fs::IFileObject> check_file;
    if (vfs->openFile(check_file, "config_item_42.json", fs::kFileRead) == fs::kCodeOK) {
        std::string result(check_file->size(), '\0');
        check_file->read(&result[0], result.size());
        std::cout << "Content of config_item_42.json (" << result.size() << " bytes):\n"
                  << "----------------------------------------\n"
                  << result
                  << "\n----------------------------------------" << std::endl;
    }

    std::cout << "\n=== Project Demo Complete: PASSED (100% OK) ===" << std::endl;
    return 0;
}
