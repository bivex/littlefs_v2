#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <limits>
#include "lfs_interface.h"

static int g_edge_passed = 0;
static int g_edge_failed = 0;

#define EDGE_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            g_edge_passed++; \
        } else { \
            g_edge_failed++; \
            std::cerr << "  [FAIL] Edge Case " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl; \
        } \
    } while (0)

void test_zero_byte_files(std::shared_ptr<fs::IFileSystemDevice>& vfs) {
    std::cout << "[Edge Case 1] Zero-Byte Files & Empty I/O..." << std::endl;

    std::shared_ptr<fs::IFileObject> file;
    EDGE_ASSERT(vfs->openFile(file, "empty.dat", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK, "Create 0-byte file");

    EDGE_ASSERT(file->size() == 0, "0-byte size check");
    EDGE_ASSERT(file->tell() == 0, "0-byte tell check");

    char buf[16];
    EDGE_ASSERT(file->read(buf, sizeof(buf)) == 0, "Read from 0-byte file returns 0");
    EDGE_ASSERT(file->write(buf, 0) == 0, "Write 0 bytes returns 0");
    file->flush();

    EDGE_ASSERT(file->size() == 0, "Size remains 0 after 0-byte write");
    vfs->deleteFile("empty.dat");
}

void test_boundary_seeks(std::shared_ptr<fs::IFileSystemDevice>& vfs) {
    std::cout << "[Edge Case 2] Boundary Seeks & Extreme Offsets..." << std::endl;

    std::shared_ptr<fs::IFileObject> file;
    vfs->openFile(file, "seek_boundary.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);

    std::string text = "0123456789";
    file->write(text.data(), text.size());
    file->flush();

    // Seek way past EOF
    EDGE_ASSERT(file->seek(1000, fs::IFileObject::kSeekSet) == 1000, "Seek past EOF to 1000");
    EDGE_ASSERT(file->tell() == 1000, "Tell at 1000");

    char buf[10];
    EDGE_ASSERT(file->read(buf, sizeof(buf)) == 0, "Read past EOF returns 0");

    // Write at offset 1000 (sparse file expansion)
    std::string tail = "TAIL";
    file->write(tail.data(), tail.size());
    file->flush();

    EDGE_ASSERT(file->size() == 1004, "File size auto-expands to 1004");

    // Seek back to 0
    file->seek(0, fs::IFileObject::kSeekSet);
    std::vector<char> full_read(1004, 0);
    EDGE_ASSERT(file->read(full_read.data(), full_read.size()) == 1004, "Read full expanded file");
    EDGE_ASSERT(std::memcmp(full_read.data(), "0123456789", 10) == 0, "Start content intact");
    EDGE_ASSERT(std::memcmp(full_read.data() + 1000, "TAIL", 4) == 0, "Tail content intact");

    vfs->deleteFile("seek_boundary.bin");
}

void test_unaligned_page_boundary_writes(std::shared_ptr<fs::IFileSystemDevice>& vfs) {
    std::cout << "[Edge Case 3] Unaligned Writes Spanning RAM Page Boundaries (4096 B)..." << std::endl;

    std::shared_ptr<fs::IFileObject> file;
    vfs->openFile(file, "page_straddle.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);

    std::vector<uint8_t> base_data(8192, 0xAA);
    file->write(base_data.data(), base_data.size());
    file->flush();

    // Write 10 bytes straddling Page 0 (0..4095) and Page 1 (4096..8191) at offset 4091
    uint8_t straddle[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    file->seek(4091, fs::IFileObject::kSeekSet);
    EDGE_ASSERT(file->write(straddle, sizeof(straddle)) == 10, "Write 10 bytes across page boundary [4091..4100]");
    file->flush();

    std::vector<uint8_t> read_back(8192);
    file->seek(0, fs::IFileObject::kSeekSet);
    file->read(read_back.data(), read_back.size());

    EDGE_ASSERT(read_back[4090] == 0xAA, "Byte before straddle unchanged");
    EDGE_ASSERT(std::memcmp(read_back.data() + 4091, straddle, 10) == 0, "Straddled page bytes 100% correct");
    EDGE_ASSERT(read_back[4101] == 0xAA, "Byte after straddle unchanged");

    vfs->deleteFile("page_straddle.bin");
}

void test_create_flags_and_missing_files(std::shared_ptr<fs::IFileSystemDevice>& vfs) {
    std::cout << "[Edge Case 4] Open Flags & Missing File Error Handling..." << std::endl;

    std::shared_ptr<fs::IFileObject> file;
    // 1. Read non-existent file without create flag
    EDGE_ASSERT(vfs->openFile(file, "non_existent.txt", fs::kFileRead) == fs::kCodeFileNotFound, "Open missing file returns kCodeFileNotFound");

    // 2. Create file with kFileCreateIfNotExists
    EDGE_ASSERT(vfs->openFile(file, "flag_test.txt", fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK, "Create file");
    file.reset();

    // 3. Open with kFileCreateFailIfExists on existing file -> should fail
    EDGE_ASSERT(vfs->openFile(file, "flag_test.txt", fs::kFileWrite | fs::kFileCreateFailIfExists) != fs::kCodeOK, "CreateFailIfExists fails on existing file");

    vfs->deleteFile("flag_test.txt");
}

void test_truncation_edge_cases(std::shared_ptr<fs::IFileSystemDevice>& vfs) {
    std::cout << "[Edge Case 5] Truncation Shrink & Expand..." << std::endl;

    std::shared_ptr<fs::IFileObject> file;
    vfs->openFile(file, "trunc_edge.bin", fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists);

    std::string text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    file->write(text.data(), text.size());
    file->flush();

    // Seek to end (26)
    file->seek(26, fs::IFileObject::kSeekSet);

    // Truncate to 10
    EDGE_ASSERT(file->truncate(10) == fs::kCodeOK, "Truncate to 10");
    EDGE_ASSERT(file->size() == 10, "Size is 10");
    EDGE_ASSERT(file->tell() == 10, "Tell adjusted to 10 from 26");

    // Truncate expand to 50
    EDGE_ASSERT(file->truncate(50) == fs::kCodeOK, "Expand truncate to 50");
    EDGE_ASSERT(file->size() == 50, "Size is 50");

    vfs->deleteFile("trunc_edge.bin");
}

void run_edge_cases() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "         littlefs_v2 Edge Cases Unit Test Suite           " << std::endl;
    std::cout << "==========================================================" << std::endl;

    std::shared_ptr<fs::IFileSystemDevice> vfs_file;
    assert(fs::createVFS(L"edge_test.fs", vfs_file, fs::lfsVFS::Backend::kFileBackend) == fs::kCodeOK);

    test_zero_byte_files(vfs_file);
    test_boundary_seeks(vfs_file);
    test_unaligned_page_boundary_writes(vfs_file);
    test_create_flags_and_missing_files(vfs_file);
    test_truncation_edge_cases(vfs_file);

    std::shared_ptr<fs::IFileSystemDevice> vfs_mem;
    assert(fs::createVFS(L"edge_test_mem.fs", vfs_mem, fs::lfsVFS::Backend::kMemoryBackend) == fs::kCodeOK);

    test_zero_byte_files(vfs_mem);
    test_boundary_seeks(vfs_mem);
    test_unaligned_page_boundary_writes(vfs_mem);
    test_create_flags_and_missing_files(vfs_mem);
    test_truncation_edge_cases(vfs_mem);

    std::cout << "\n----------------------------------------------------------" << std::endl;
    std::cout << "EDGE CASE SUMMARY: Passed: " << g_edge_passed << " | Failed: " << g_edge_failed << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    assert(g_edge_failed == 0);
}

int main() {
    run_edge_cases();
    return 0;
}
