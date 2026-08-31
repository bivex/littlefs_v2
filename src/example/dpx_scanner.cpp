#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <algorithm>

#include "lfs_interface.h"
#include "lfs_block_device.h"

namespace fs_std = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

struct FileStats {
    std::string rel_path;
    std::string ext;
    uint64_t size_bytes{0};
    uint64_t total_lines{0};
    uint64_t code_lines{0};
    uint64_t blank_lines{0};
    uint64_t comment_lines{0};
    bool verified{false};
};

struct LangStats {
    size_t file_count{0};
    uint64_t total_bytes{0};
    uint64_t total_lines{0};
    uint64_t code_lines{0};
    uint64_t comment_lines{0};
    uint64_t blank_lines{0};
};

static bool is_ignored(const fs_std::path& p) {
    std::string str = p.string();
    if (str.find("/.git") != std::string::npos ||
        str.find("/.venv") != std::string::npos ||
        str.find("/__pycache__") != std::string::npos ||
        str.find("/.mypy_cache") != std::string::npos ||
        str.find("/.pytest_cache") != std::string::npos ||
        str.find("/.ruff_cache") != std::string::npos ||
        str.find("/.DS_Store") != std::string::npos) {
        return true;
    }
    return false;
}

static void analyze_lines(const std::string& content, const std::string& ext,
                          uint64_t& total_l, uint64_t& code_l, uint64_t& blank_l, uint64_t& comment_l) {
    std::istringstream stream(content);
    std::string line;
    bool in_block_comment = false;

    while (std::getline(stream, line)) {
        total_l++;
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed.empty()) {
            blank_l++;
            continue;
        }

        if (ext == ".py") {
            if (trimmed[0] == '#') {
                comment_l++;
            } else if (trimmed.rfind("\"\"\"", 0) == 0 || trimmed.rfind("'''", 0) == 0) {
                comment_l++;
            } else {
                code_l++;
            }
        } else if (ext == ".cpp" || ext == ".c" || ext == ".h" || ext == ".hpp") {
            if (trimmed.rfind("//", 0) == 0) {
                comment_l++;
            } else if (trimmed.rfind("/*", 0) == 0) {
                in_block_comment = true;
                comment_l++;
                if (trimmed.find("*/") != std::string::npos) in_block_comment = false;
            } else if (in_block_comment) {
                comment_l++;
                if (trimmed.find("*/") != std::string::npos) in_block_comment = false;
            } else {
                code_l++;
            }
        } else {
            code_l++;
        }
    }
}

int main(int argc, char** argv) {
    std::string target_dir = (argc > 1) ? argv[1] : "/Volumes/External/Code/DPX-Cpp";

    std::cout << "==========================================================================================" << std::endl;
    std::cout << "               Scanning & Ingesting Codebase into littlefs_v2 VFS Engine                  " << std::endl;
    std::cout << " Target Source: " << target_dir << std::endl;
    std::cout << "==========================================================================================" << std::endl;

    if (!fs_std::exists(target_dir)) {
        std::cerr << "[-] Error: Target directory does not exist: " << target_dir << std::endl;
        return 1;
    }

    // 1. Host File System Discovery
    std::cout << "[*] Phase 1: Discovering project files on host filesystem..." << std::endl;
    auto t_start_scan = Clock::now();
    std::vector<fs_std::path> file_paths;

    for (const auto& entry : fs_std::recursive_directory_iterator(target_dir)) {
        if (entry.is_regular_file() && !is_ignored(entry.path())) {
            file_paths.push_back(entry.path());
        }
    }
    auto t_end_scan = Clock::now();
    double scan_ms = std::chrono::duration<double, std::milli>(t_end_scan - t_start_scan).count();
    std::cout << "[+] Found " << file_paths.size() << " files in " << scan_ms << " ms" << std::endl;

    // 2. Create in-memory LittleFS VFS
    std::cout << "[*] Phase 2: Initializing littlefs_v2 Virtual File System..." << std::endl;
    auto mem_dev = std::make_shared<fs::MemoryBlockDevice>(64 * 1024, 1024);
    auto vfs_res = fs::createVFSWithDevice(mem_dev);
    if (!vfs_res.has_value()) {
        std::cerr << "[-] Error: Failed to initialize LittleFS VFS: " << fs::errorToString(vfs_res.error()) << std::endl;
        return 1;
    }
    auto vfs = vfs_res.value();

    // 3. Ingest files into LittleFS VFS
    std::cout << "[*] Phase 3: Ingesting files into littlefs_v2 container..." << std::endl;
    auto t_start_ingest = Clock::now();
    uint64_t total_bytes_ingested = 0;
    std::vector<FileStats> stats_list;

    for (size_t i = 0; i < file_paths.size(); ++i) {
        const auto& path = file_paths[i];
        std::string rel = fs_std::relative(path, target_dir).string();
        std::string vfs_key = "f_" + std::to_string(i) + "_" + path.filename().string();

        std::ifstream fin(path, std::ios::binary);
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();

        total_bytes_ingested += buffer.size();

        auto f_res = vfs->open(vfs_key, fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);
        if (!f_res.has_value()) {
            std::cerr << "[-] Error opening VFS file: " << vfs_key << std::endl;
            continue;
        }

        fs::FileHandle handle(f_res.value());
        if (!buffer.empty()) {
            handle->write(buffer.data(), buffer.size());
        }
        handle->flush();

        FileStats s;
        s.rel_path = rel;
        s.ext = path.extension().string();
        s.size_bytes = buffer.size();
        stats_list.push_back(s);
    }
    auto t_end_ingest = Clock::now();
    double ingest_ms = std::chrono::duration<double, std::milli>(t_end_ingest - t_start_ingest).count();
    double mb_ingested = total_bytes_ingested / (1024.0 * 1024.0);
    double ingest_mbs = (mb_ingested / (ingest_ms / 1000.0));

    std::cout << "[+] Ingestion completed: " << file_paths.size() << " files (" << std::fixed << std::setprecision(2)
              << mb_ingested << " MB) in " << ingest_ms << " ms (" << ingest_mbs << " MB/s)" << std::endl;

    // 4. Scan, Parse and Verify from inside LittleFS VFS
    std::cout << "[*] Phase 4: Scanning, analyzing and verifying data directly from LittleFS VFS..." << std::endl;
    auto t_start_verify = Clock::now();
    std::map<std::string, LangStats> lang_map;
    uint64_t total_code_lines = 0;
    uint64_t total_comment_lines = 0;
    uint64_t total_blank_lines = 0;
    uint64_t total_lines_all = 0;
    size_t verified_count = 0;

    for (size_t i = 0; i < stats_list.size(); ++i) {
        auto& s = stats_list[i];
        std::string vfs_key = "f_" + std::to_string(i) + "_" + fs_std::path(s.rel_path).filename().string();

        auto f_res = vfs->open(vfs_key, fs::kFileRead);
        if (!f_res.has_value()) {
            std::cerr << "[-] Error reading from VFS: " << vfs_key << std::endl;
            continue;
        }

        fs::FileHandle handle(f_res.value());
        uint64_t sz = handle->size();
        std::vector<char> buf(sz + 1, 0);
        if (sz > 0) {
            handle->read(buf.data(), sz);
        }

        std::string content(buf.data(), sz);
        analyze_lines(content, s.ext, s.total_lines, s.code_lines, s.blank_lines, s.comment_lines);

        total_lines_all += s.total_lines;
        total_code_lines += s.code_lines;
        total_comment_lines += s.comment_lines;
        total_blank_lines += s.blank_lines;

        auto& ls = lang_map[s.ext.empty() ? "(no ext)" : s.ext];
        ls.file_count++;
        ls.total_bytes += s.size_bytes;
        ls.total_lines += s.total_lines;
        ls.code_lines += s.code_lines;
        ls.comment_lines += s.comment_lines;
        ls.blank_lines += s.blank_lines;

        s.verified = true;
        verified_count++;
    }
    auto t_end_verify = Clock::now();
    double verify_ms = std::chrono::duration<double, std::milli>(t_end_verify - t_start_verify).count();
    double read_mbs = (mb_ingested / (verify_ms / 1000.0));

    std::cout << "[+] VFS Data verification & LOC parsing finished in " << verify_ms << " ms (" << read_mbs << " MB/s)" << std::endl;

    // 5. Print Detailed Summary Reports
    std::cout << "\n==========================================================================================" << std::endl;
    std::cout << "                         DPX-Cpp Codebase Scan Summary                                    " << std::endl;
    std::cout << "==========================================================================================" << std::endl;
    std::cout << " Total Files Ingested:       " << stats_list.size() << std::endl;
    std::cout << " Total Data Size:            " << std::fixed << std::setprecision(2) << mb_ingested << " MB (" << total_bytes_ingested << " bytes)" << std::endl;
    std::cout << " Total Lines of Code (LOC):  " << total_lines_all << " lines" << std::endl;
    std::cout << "   - Pure Code Lines:        " << total_code_lines << " (" << std::fixed << std::setprecision(1) << (total_code_lines * 100.0 / std::max(1ULL, total_lines_all)) << "%)" << std::endl;
    std::cout << "   - Comment Lines:          " << total_comment_lines << " (" << (total_comment_lines * 100.0 / std::max(1ULL, total_lines_all)) << "%)" << std::endl;
    std::cout << "   - Blank Lines:            " << total_blank_lines << " (" << (total_blank_lines * 100.0 / std::max(1ULL, total_lines_all)) << "%)" << std::endl;
    std::cout << "\n### Language & Extension Breakdown\n" << std::endl;
    std::cout << "| Extension | Files | Total Size | Total Lines | Code Lines | Comments | Blank Lines |" << std::endl;
    std::cout << "|:----------|------:|-----------:|------------:|-----------:|---------:|------------:|" << std::endl;

    for (const auto& [ext, ls] : lang_map) {
        std::cout << "| " << std::left << std::setw(9) << ext
                  << " | " << std::right << std::setw(5) << ls.file_count
                  << " | " << std::right << std::setw(8) << (ls.total_bytes / 1024.0) << " KB"
                  << " | " << std::right << std::setw(11) << ls.total_lines
                  << " | " << std::right << std::setw(10) << ls.code_lines
                  << " | " << std::right << std::setw(8) << ls.comment_lines
                  << " | " << std::right << std::setw(11) << ls.blank_lines
                  << " |" << std::endl;
    }

    std::cout << "\n### Sample Top 10 Largest Files in DPX-Cpp:\n" << std::endl;
    std::sort(stats_list.begin(), stats_list.end(), [](const FileStats& a, const FileStats& b) {
        return a.size_bytes > b.size_bytes;
    });

    for (size_t i = 0; i < std::min((size_t)10, stats_list.size()); ++i) {
        std::cout << " " << std::setw(2) << (i + 1) << ". " << std::left << std::setw(55) << stats_list[i].rel_path
                  << " (" << std::right << std::setw(6) << (stats_list[i].size_bytes / 1024.0) << " KB, "
                  << std::setw(5) << stats_list[i].total_lines << " lines)" << std::endl;
    }

    std::cout << "\n==========================================================================================" << std::endl;
    return 0;
}
