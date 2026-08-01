#include "lfs_interface.h"
#include "file_backend.h"
#include "memory_backend.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>

#ifndef _WIN32
#include <codecvt>
#include <locale>
static inline int _wfopen_s(FILE** pFile, const wchar_t* filename, const wchar_t* mode) {
    if (!pFile) return -1;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string path = conv.to_bytes(filename);
    std::string m = conv.to_bytes(mode);
    FILE* f = fopen(path.c_str(), m.c_str());
    if (!f) {
        *pFile = nullptr;
        return -1;
    }
    *pFile = f;
    return 0;
}
#endif

namespace fs {

static ErrorCode lfsToHxErrorCode(int err) {

    switch (err) {
    case LFS_ERR_OK: { // No error
        return ErrorCode::kCodeOK;
    }
    case LFS_ERR_IO: // Error during device operation
    case LFS_ERR_CORRUPT: // Corrupted
    case LFS_ERR_BADF: {// Bad file number
        return ErrorCode::kCodeBadDevice;
    }

    case LFS_ERR_FBIG:  // File too large
    case LFS_ERR_NOSPC: { // No space left on device
        return ErrorCode::kCodeNoDeviceSpace;
    }

    case LFS_ERR_INVAL: { // Invalid parameter
        return ErrorCode::kCodeObjectNotCompatible;
    }

    case LFS_ERR_NOENT: { // No such file or directory
        return ErrorCode::kCodeFileNotFound;
    }

    default: {
        return ErrorCode::kCodeUnknownError;
    }
    }
}

lfsVFS::lfsVFS(
    std::shared_ptr<lfs_t> lfs_handle,
    std::shared_ptr<lfs_config_t> lfs_config,
    std::shared_ptr<VFSContext> lfs_context) {

    _lfs_handle = lfs_handle;
    _lfs_config = lfs_config;
    _lfs_context = lfs_context;
}

lfsVFS::~lfsVFS() {
    lfs_unmount(_lfs_handle.get());
}

struct VFSFileObject
    : public IFileObject {
    std::shared_ptr<lfs_t> _lfs_handle;
    lfs_file_t _file_handle;
    uint64_t _pos;
    uint64_t _file_size;
    bool _is_open;

    static constexpr size_t PAGE_SIZE = 4096;

    struct Page {
        std::vector<uint8_t> data;
        bool dirty = false;
    };
    std::unordered_map<uint64_t, Page> _pages;

    VFSFileObject(std::shared_ptr<lfs_t> lfs_handle)
        : _lfs_handle(lfs_handle)
        , _file_handle({})
        , _pos(0)
        , _file_size(0)
        , _is_open(false) {}

    void init_file_state() {
        _pos = 0;
        lfs_soff_t sz = lfs_file_size(_lfs_handle.get(), &_file_handle);
        _file_size = (sz >= 0) ? (uint64_t)sz : 0;
        _is_open = true;
    }

    ~VFSFileObject() {
        if (_is_open) {
            flush();
            lfs_file_close(_lfs_handle.get(), &_file_handle);
            _is_open = false;
        }
    }

private:
    void load_page_if_needed(uint64_t page_idx) {
        auto& page = _pages[page_idx];
        if (page.data.empty()) {
            page.data.resize(PAGE_SIZE, 0);
            uint64_t page_start = page_idx * PAGE_SIZE;
            lfs_soff_t disk_sz = lfs_file_size(_lfs_handle.get(), &_file_handle);
            if (disk_sz > 0 && page_start < (uint64_t)disk_sz) {
                uint64_t to_read = std::min((uint64_t)PAGE_SIZE, (uint64_t)disk_sz - page_start);
                lfs_file_seek(_lfs_handle.get(), &_file_handle, page_start, LFS_SEEK_SET);
                lfs_file_read(_lfs_handle.get(), &_file_handle, page.data.data(), to_read);
            }
        }
    }

public:
    int64_t read(void* data, uint64_t size) override {
        if (!_is_open || size == 0) return 0;
        if (_pos >= _file_size) return 0;

        uint64_t bytes_to_read = std::min(size, _file_size - _pos);
        uint64_t bytes_read = 0;
        uint8_t* dst = static_cast<uint8_t*>(data);

        while (bytes_read < bytes_to_read) {
            uint64_t curr_pos = _pos + bytes_read;
            uint64_t page_idx = curr_pos / PAGE_SIZE;
            uint64_t page_off = curr_pos % PAGE_SIZE;
            uint64_t chunk = std::min(bytes_to_read - bytes_read, (uint64_t)(PAGE_SIZE - page_off));

            auto it = _pages.find(page_idx);
            if (it != _pages.end() && !it->second.data.empty()) {
                std::memcpy(dst + bytes_read, it->second.data.data() + page_off, chunk);
            } else {
                lfs_file_seek(_lfs_handle.get(), &_file_handle, curr_pos, LFS_SEEK_SET);
                lfs_ssize_t res = lfs_file_read(_lfs_handle.get(), &_file_handle, dst + bytes_read, chunk);
                if (res < 0) {
                    if (bytes_read > 0) break;
                    return res;
                }
            }
            bytes_read += chunk;
        }

        _pos += bytes_read;
        return bytes_read;
    }

    int64_t write(const void* data, uint64_t size) override {
        if (!_is_open || size == 0) return 0;

        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint64_t bytes_written = 0;

        while (bytes_written < size) {
            uint64_t curr_pos = _pos + bytes_written;
            uint64_t page_idx = curr_pos / PAGE_SIZE;
            uint64_t page_off = curr_pos % PAGE_SIZE;
            uint64_t chunk = std::min(size - bytes_written, (uint64_t)(PAGE_SIZE - page_off));

            load_page_if_needed(page_idx);
            auto& page = _pages[page_idx];

            std::memcpy(page.data.data() + page_off, src + bytes_written, chunk);
            page.dirty = true;

            bytes_written += chunk;
            if (curr_pos + chunk > _file_size) {
                _file_size = curr_pos + chunk;
            }
        }

        _pos += bytes_written;
        return bytes_written;
    }

    int64_t truncate(uint64_t size) override {
        if (!_is_open) return LFS_ERR_BADF;

        flush();

        int err = lfs_file_truncate(_lfs_handle.get(), &_file_handle, size);
        if (err == LFS_ERR_OK) {
            _file_size = size;
            if (_pos > _file_size) {
                _pos = _file_size;
            }
            uint64_t max_page = (size + PAGE_SIZE - 1) / PAGE_SIZE;
            for (auto it = _pages.begin(); it != _pages.end();) {
                if (it->first >= max_page) {
                    it = _pages.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return err;
    }

    int64_t seek(uint64_t offset, SeekType type) override {
        if (!_is_open) return LFS_ERR_BADF;

        int64_t new_pos = _pos;
        switch (type) {
        case kSeekSet: new_pos = offset; break;
        case kSeekCur: new_pos = (int64_t)_pos + offset; break;
        case kSeekEnd: new_pos = (int64_t)_file_size + offset; break;
        }

        if (new_pos < 0) return LFS_ERR_INVAL;
        _pos = static_cast<uint64_t>(new_pos);
        return _pos;
    }

    int64_t tell() override {
        return _pos;
    }

    int64_t size() override {
        return _file_size;
    }

    void flush() override {
        if (!_is_open) return;

        std::vector<uint64_t> dirty_indices;
        for (const auto& kv : _pages) {
            if (kv.second.dirty) {
                dirty_indices.push_back(kv.first);
            }
        }

        if (dirty_indices.empty()) return;

        std::sort(dirty_indices.begin(), dirty_indices.end());

        for (uint64_t page_idx : dirty_indices) {
            auto& page = _pages[page_idx];
            if (!page.dirty) continue;

            uint64_t page_start = page_idx * PAGE_SIZE;
            uint64_t write_bytes = std::min((uint64_t)PAGE_SIZE, _file_size - page_start);

            if (write_bytes > 0) {
                lfs_file_seek(_lfs_handle.get(), &_file_handle, page_start, LFS_SEEK_SET);
                lfs_file_write(_lfs_handle.get(), &_file_handle, page.data.data(), write_bytes);
            }
            page.dirty = false;
        }

        lfs_file_sync(_lfs_handle.get(), &_file_handle);
    }
};

std::vector<Entry> lfsVFS::dir(const std::string& path) {

    std::vector<Entry> entries;

    lfs_dir_t dir;

    int err = lfs_dir_open(_lfs_handle.get(), &dir, path.c_str());

    if (err) {
        return entries;
    }

    struct lfs_info info;

    while (true) {

        int res = lfs_dir_read(_lfs_handle.get(), &dir, &info);

        if (res < 0) {
            break;
        }

        switch (info.type) {
        case LFS_TYPE_REG: {
            entries.push_back(FileEntry(info.name, info.size));
            break;
        }

        case LFS_TYPE_DIR: {

            if (std::string(info.name) == "." ||
                std::string(info.name) == "..") {

                continue;
            }

            entries.push_back(DirectoryEntry(info.name, info.size));
            break;
        }
        }
    }

    lfs_dir_close(_lfs_handle.get(), &dir);
    return entries;
}

ErrorCode lfsVFS::openFile(std::shared_ptr<IFileObject>& handle, const std::string& path, uint32_t flags) {

    uint32_t lfs_flags = 0;

    if (flags & kFileRead) { lfs_flags |= LFS_O_RDONLY; }
    if (flags & kFileWrite) { lfs_flags |= LFS_O_WRONLY; }
    if (flags & kFileCreateIfNotExists) { lfs_flags |= LFS_O_CREAT; }
    if (flags & kFileCreateFailIfExists) { lfs_flags |= LFS_O_EXCL; }
    if (flags & kFileTruncate) { lfs_flags |= LFS_O_TRUNC; }
    if (flags & kFileAppend) { lfs_flags |= LFS_O_APPEND; }

    auto file_obj = std::make_shared<VFSFileObject>(_lfs_handle);

    int err = lfs_file_open(
        _lfs_handle.get(),
        &file_obj->_file_handle,
        path.c_str(), lfs_flags);

    file_obj->_path = path;

    if (err != LFS_ERR_OK) {
        return lfsToHxErrorCode(err);
    }

    file_obj->init_file_state();
    handle = file_obj;

    return lfsToHxErrorCode(err);
}

ErrorCode lfsVFS::existsFile(const std::string& path) {

    lfs_file_t _file_handle;

    int err = lfs_file_open(
        _lfs_handle.get(),
        &_file_handle,
        path.c_str(), LFS_O_RDONLY);

    if (err == LFS_ERR_OK) {

        lfs_file_close(_lfs_handle.get(), &_file_handle);
        return ErrorCode::kCodeOK;
    }

    return ErrorCode::kCodeFileNotFound;
}

ErrorCode lfsVFS::deleteFile(const std::string& path) {
    return lfsToHxErrorCode(lfs_remove(_lfs_handle.get(), path.c_str()));
}

ErrorCode lfsVFS::deleteDirectory(const std::string& path) {
    return lfsToHxErrorCode(lfs_remove(_lfs_handle.get(), path.c_str()));
}

ErrorCode openVFS(const std::wstring& path, std::shared_ptr< IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {

    std::shared_ptr< lfs_t> fs_handle(new lfs_t);
    std::shared_ptr< lfs_config_t> fs_config(new lfs_config_t);
    std::shared_ptr< lfsVFS::VFSContext> fs_context;

    if (backend == lfsVFS::Backend::kFileBackend) {

        FILE* file = nullptr;
        _wfopen_s(&file, path.c_str(), L"r+b");

        if (!file) {
            return ErrorCode::kCodeFileNotFound;
        }

        std::shared_ptr<FILE> file_handle(file, 
            [](FILE* file) {
                fclose(file);
            });

        fs_context = std::shared_ptr< lfsVFS::VFSContext>(
            new vfs_file_context(
                fs_handle,
                file_handle
            )
        );
    }
    else if (backend == lfsVFS::Backend::kMemoryBackend) {
        fs_context = std::shared_ptr< lfsVFS::VFSContext>(new vfs_memory_context(fs_handle));
    }
    else {
        return ErrorCode::kCodeObjectNotCompatible;
    }

    std::shared_ptr< IFileSystemDevice> _fs(
        std::make_shared<lfsVFS>(
            fs_handle,
            fs_config,
            fs_context
        )
    );

    {
        lfs_t* handle = fs_handle.get();

        memset(handle, 0, sizeof(lfs_t));
    }

    { //setup config
        lfs_config_t* config = fs_config.get();

        memset(config, 0, sizeof(lfs_config_t));

        config->context = fs_context.get();

        // block device operations
        if (backend == lfsVFS::Backend::kFileBackend) {
            config->read = vfs_file_block_device_read;
            config->write = vfs_file_block_device_prog;
            config->erase = vfs_file_block_device_erase;
            config->sync = vfs_file_block_device_sync;
            config->allocate_block = vfs_file_allocate_block;
            config->lock = 0;
            config->unlock = 0;
        }
        else if (backend == lfsVFS::Backend::kMemoryBackend) {
            config->read = vfs_memory_block_device_read;
            config->write = vfs_memory_block_device_prog;
            config->erase = vfs_memory_block_device_erase;
            config->sync = vfs_memory_block_device_sync;
            config->allocate_block = vfs_memory_allocate_block;
            config->lock = 0;
            config->unlock = 0;
        }

        // block device configuration
        config->read_size = 1;
        config->write_size = 1;
        config->block_size = (1024 * 64);
        config->block_count = 2;
        config->cache_size = config->block_size;
        config->erase_size = 0;
        config->lookahead_size = config->block_size;
        config->block_cycles = -1;
        config->file_max_size = 0x7fffffffffffffff;
        config->on_grow = false;
    }

    //setup context
    if (backend == lfsVFS::Backend::kFileBackend) {

        vfs_file_context* context = (vfs_file_context*)fs_context.get();
        lfs_config_t* config = fs_config.get();

        context->_lfs_handle = fs_handle;
    }
    else if (backend == lfsVFS::Backend::kMemoryBackend) {

        vfs_memory_context* context = (vfs_memory_context*)fs_context.get();
        lfs_config_t* config = fs_config.get();

        context->_lfs_handle = fs_handle;
    }

    {
        int err = lfs_mount(fs_handle.get(),fs_config.get());

        if (err) {

            return lfsToHxErrorCode(err);
        }
    }

    filesystem = _fs;

    return ErrorCode::kCodeOK;
}

ErrorCode createVFS(const std::wstring& path, std::shared_ptr< IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {

    std::shared_ptr< lfs_t> fs_handle(new lfs_t);
    std::shared_ptr< lfs_config_t> fs_config(new lfs_config_t);
    std::shared_ptr< lfsVFS::VFSContext> fs_context;

    if (backend == lfsVFS::Backend::kFileBackend) {

        FILE* file = nullptr;
        _wfopen_s(&file, path.c_str(), L"w+b");

        if (!file) {
            return ErrorCode::kCodeFileNotFound;
        }

        std::shared_ptr<FILE> file_handle(file, [](FILE* file) {
            fclose(file);
            });

        fs_context = std::shared_ptr< lfsVFS::VFSContext>(
            new vfs_file_context(
                fs_handle,
                file_handle
            )
        );
    }
    else if (backend == lfsVFS::Backend::kMemoryBackend) {
        fs_context = std::shared_ptr< lfsVFS::VFSContext>(new vfs_memory_context(fs_handle));
    }
    else {
        return ErrorCode::kCodeObjectNotCompatible;
    }

    std::shared_ptr< IFileSystemDevice> _fs(
        std::make_shared<lfsVFS>(
            fs_handle,
            fs_config,
            fs_context
        )
    );

    {
        lfs_t* handle = fs_handle.get();

        memset(handle, 0, sizeof(lfs_t));
    }

    { //setup config
        lfs_config_t* config = fs_config.get();

        memset(config, 0, sizeof(lfs_config_t));

        config->context = fs_context.get();

        // block device operations
        if (backend == lfsVFS::Backend::kFileBackend) {
            config->read = vfs_file_block_device_read;
            config->write = vfs_file_block_device_prog;
            config->erase = vfs_file_block_device_erase;
            config->sync = vfs_file_block_device_sync;
            config->allocate_block = vfs_file_allocate_block;
            config->lock = 0;
            config->unlock = 0;
        }
        else if (backend == lfsVFS::Backend::kMemoryBackend) {
            config->read = vfs_memory_block_device_read;
            config->write = vfs_memory_block_device_prog;
            config->erase = vfs_memory_block_device_erase;
            config->sync = vfs_memory_block_device_sync;
            config->allocate_block = vfs_memory_allocate_block;
            config->lock = 0;
            config->unlock = 0;
        }

        // block device configuration
        config->read_size = 1;
        config->write_size = 1;
        config->block_size = (1024 * 64);
        config->block_count = 2;
        config->cache_size = config->block_size;
        config->erase_size = 0;
        config->lookahead_size = config->block_size;
        config->block_cycles = -1;
        config->file_max_size = 0x7fffffffffffffff;
        config->on_grow = false;
    }

    //setup context
    if (backend == lfsVFS::Backend::kFileBackend) {

        vfs_file_context* context = (vfs_file_context*)fs_context.get();
        lfs_config_t* config = fs_config.get();

        context->_lfs_handle = fs_handle;

        vfs_file_allocate_file_size(config, 2);
    }
    else if (backend == lfsVFS::Backend::kMemoryBackend) {

        vfs_memory_context* context = (vfs_memory_context*)fs_context.get();
        lfs_config_t* config = fs_config.get();

        context->_lfs_handle = fs_handle;

        vfs_memory_allocate_file_size(config, 2);
    }

    {
        int err = lfs_format(fs_handle.get(), fs_config.get());

        if (err) {

            return lfsToHxErrorCode(err);
        }

        err = lfs_mount(fs_handle.get(),fs_config.get());

        if (err) {

            return lfsToHxErrorCode(err);
        }
    }

    filesystem = _fs;

    return ErrorCode::kCodeOK;
}
} // namespace fs