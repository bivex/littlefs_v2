#include "lfs_interface.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <codecvt>
#include <locale>

namespace fs {

    struct ErrorMapEntry {
        int lfs_err;
        ErrorCode modern_err;
        LegacyErrorCode legacy_err;
    };

    static constexpr ErrorMapEntry kErrorMap[] = {
        { LFS_ERR_OK, ErrorCode::kSuccess, kCodeOK },
        { LFS_ERR_NOENT, ErrorCode::kFileNotFound, kCodeFileNotFound },
        { LFS_ERR_EXIST, ErrorCode::kFileExists, kCodeUnknownError },
        { LFS_ERR_NOTDIR, ErrorCode::kNotDirectory, kCodeObjectNotCompatible },
        { LFS_ERR_ISDIR, ErrorCode::kIsDirectory, kCodeObjectNotCompatible },
        { LFS_ERR_NOTEMPTY, ErrorCode::kDirectoryNotEmpty, kCodeUnknownError },
        { LFS_ERR_BADF, ErrorCode::kBadFileDescriptor, kCodeBadDevice },
        { LFS_ERR_FBIG, ErrorCode::kFileTooLarge, kCodeNoDeviceSpace },
        { LFS_ERR_INVAL, ErrorCode::kInvalidParameter, kCodeObjectNotCompatible },
        { LFS_ERR_NOSPC, ErrorCode::kNoSpaceOnDevice, kCodeNoDeviceSpace },
        { LFS_ERR_NOMEM, ErrorCode::kNoMemory, kCodeUnknownError },
        { LFS_ERR_NOATTR, ErrorCode::kNoAttribute, kCodeUnknownError },
        { LFS_ERR_NAMETOOLONG, ErrorCode::kNameTooLong, kCodeUnknownError },
        { LFS_ERR_CORRUPT, ErrorCode::kCorrupted, kCodeBadDevice },
        { LFS_ERR_IO, ErrorCode::kIoError, kCodeBadDevice },
    };

    static ErrorCode lfsToErrorCode(int err) {
        for (const auto& entry : kErrorMap) {
            if (entry.lfs_err == err) return entry.modern_err;
        }
        return ErrorCode::kUnknownError;
    }

    static LegacyErrorCode lfsToLegacyErrorCode(int err) {
        for (const auto& entry : kErrorMap) {
            if (entry.lfs_err == err) return entry.legacy_err;
        }
        return kCodeUnknownError;
    }

    // Bridge callbacks connecting LittleFS C function pointers to C++ IBlockDevice
    static int lfs_bd_cb_read(const lfs_config_t* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        return ctx->block_device->read(block, off, buffer, size);
    }

    static int lfs_bd_cb_prog(const lfs_config_t* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        return ctx->block_device->write(block, off, buffer, size);
    }

    static int lfs_bd_cb_erase(const lfs_config_t* c, lfs_block_t block) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        return ctx->block_device->erase(block);
    }

    static int lfs_bd_cb_sync(const lfs_config_t* c) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        return ctx->block_device->sync();
    }

    static int lfs_bd_cb_alloc(lfs_config_t* c) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        if (ctx->on_grow) {
            return LFS_ERR_NOSPC;
        }
        ctx->on_grow = true;
        int err = ctx->block_device->allocate_block();
        if (err == LFS_ERR_OK) {
            lfs_size_t new_count = ctx->block_device->get_block_count();
            lfs_fs_grow(ctx->lfs_handle.get(), new_count);
        }
        ctx->on_grow = false;
        return err;
    }

    static int lfs_bd_cb_lock(const lfs_config_t* c) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        ctx->mutex.lock();
        return 0;
    }

    static int lfs_bd_cb_unlock(const lfs_config_t* c) {
        auto* ctx = static_cast<lfsVFS::VFSContext*>(c->context);
        ctx->mutex.unlock();
        return 0;
    }

    struct VFSFileObject : public IFileObject {
        std::shared_ptr<lfs_t> _lfs_handle;
        std::shared_ptr<lfsVFS::VFSContext> _lfs_context;
        lfs_file_t _file_handle;
        uint64_t _pos{0};
        uint64_t _file_size{0};
        bool _is_open{false};

        static constexpr size_t PAGE_SIZE = 4096;

        struct Page {
            std::vector<uint8_t> data;
            bool dirty = false;
        };
        std::unordered_map<uint64_t, Page> _pages;

        VFSFileObject(std::shared_ptr<lfs_t> lfs_handle, std::shared_ptr<lfsVFS::VFSContext> lfs_context)
            : _lfs_handle(lfs_handle), _lfs_context(lfs_context), _file_handle({}) {}

        void init_file_state() {
            _pos = 0;
            lfs_soff_t sz = lfs_file_size(_lfs_handle.get(), &_file_handle);
            _file_size = (sz >= 0) ? static_cast<uint64_t>(sz) : 0;
            _is_open = true;
        }

        ~VFSFileObject() override {
            close();
        }

        void close() override {
            if (_is_open && _lfs_handle) {
                std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
                flush_internal();
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
                if (disk_sz > 0 && page_start < static_cast<uint64_t>(disk_sz)) {
                    uint64_t to_read = std::min(static_cast<uint64_t>(PAGE_SIZE), static_cast<uint64_t>(disk_sz) - page_start);
                    lfs_file_seek(_lfs_handle.get(), &_file_handle, static_cast<lfs_soff_t>(page_start), LFS_SEEK_SET);
                    lfs_file_read(_lfs_handle.get(), &_file_handle, page.data.data(), to_read);
                }
            }
        }

        void flush_internal() {
            if (!_is_open || !_lfs_handle) return;

            // Sort dirty pages by index for sequential write optimization
            std::vector<uint64_t> dirty_pages;
            for (const auto& kv : _pages) {
                if (kv.second.dirty) {
                    dirty_pages.push_back(kv.first);
                }
            }
            std::sort(dirty_pages.begin(), dirty_pages.end());

            for (uint64_t p_idx : dirty_pages) {
                auto& page = _pages[p_idx];
                uint64_t page_start = p_idx * PAGE_SIZE;
                if (page_start < _file_size) {
                    uint64_t to_write = std::min(static_cast<uint64_t>(PAGE_SIZE), _file_size - page_start);
                    lfs_file_seek(_lfs_handle.get(), &_file_handle, static_cast<lfs_soff_t>(page_start), LFS_SEEK_SET);
                    lfs_file_write(_lfs_handle.get(), &_file_handle, page.data.data(), to_write);
                }
                page.dirty = false;
            }

            lfs_file_sync(_lfs_handle.get(), &_file_handle);
        }

    public:
        int64_t read(void* data, uint64_t size) override {
            if (!_is_open || size == 0) return 0;
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
            if (_pos >= _file_size) return 0;

            uint64_t bytes_to_read = std::min(size, _file_size - _pos);
            uint64_t bytes_read = 0;
            uint8_t* dst = static_cast<uint8_t*>(data);

            while (bytes_read < bytes_to_read) {
                uint64_t curr_pos = _pos + bytes_read;
                uint64_t page_idx = curr_pos / PAGE_SIZE;
                uint64_t page_off = curr_pos % PAGE_SIZE;
                uint64_t chunk = std::min(bytes_to_read - bytes_read, static_cast<uint64_t>(PAGE_SIZE - page_off));

                auto it = _pages.find(page_idx);
                if (it != _pages.end() && !it->second.data.empty()) {
                    std::memcpy(dst + bytes_read, it->second.data.data() + page_off, static_cast<size_t>(chunk));
                } else {
                    lfs_file_seek(_lfs_handle.get(), &_file_handle, static_cast<lfs_soff_t>(curr_pos), LFS_SEEK_SET);
                    lfs_ssize_t res = lfs_file_read(_lfs_handle.get(), &_file_handle, dst + bytes_read, chunk);
                    if (res < 0) {
                        if (bytes_read > 0) break;
                        return res;
                    }
                }
                bytes_read += chunk;
            }

            _pos += bytes_read;
            return static_cast<int64_t>(bytes_read);
        }

        int64_t write(const void* data, uint64_t size) override {
            if (!_is_open || size == 0) return 0;
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);

            const uint8_t* src = static_cast<const uint8_t*>(data);
            uint64_t bytes_written = 0;

            while (bytes_written < size) {
                uint64_t curr_pos = _pos + bytes_written;
                uint64_t page_idx = curr_pos / PAGE_SIZE;
                uint64_t page_off = curr_pos % PAGE_SIZE;
                uint64_t chunk = std::min(size - bytes_written, static_cast<uint64_t>(PAGE_SIZE - page_off));

                load_page_if_needed(page_idx);
                auto& page = _pages[page_idx];

                std::memcpy(page.data.data() + page_off, src + bytes_written, static_cast<size_t>(chunk));
                page.dirty = true;

                bytes_written += chunk;
                if (curr_pos + chunk > _file_size) {
                    _file_size = curr_pos + chunk;
                }
            }

            _pos += bytes_written;
            return static_cast<int64_t>(bytes_written);
        }

        int64_t truncate(uint64_t size) override {
            if (!_is_open) return LFS_ERR_BADF;
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);

            flush_internal();

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
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);

            int64_t new_pos = static_cast<int64_t>(_pos);
            switch (type) {
                case kSeekSet: new_pos = static_cast<int64_t>(offset); break;
                case kSeekCur: new_pos = static_cast<int64_t>(_pos) + static_cast<int64_t>(offset); break;
                case kSeekEnd: new_pos = static_cast<int64_t>(_file_size) + static_cast<int64_t>(offset); break;
            }

            if (new_pos < 0) {
                return LFS_ERR_INVAL;
            }

            _pos = static_cast<uint64_t>(new_pos);
            return static_cast<int64_t>(_pos);
        }

        int64_t tell() override {
            if (!_is_open) return LFS_ERR_BADF;
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
            return static_cast<int64_t>(_pos);
        }

        int64_t size() override {
            if (!_is_open) return LFS_ERR_BADF;
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
            return static_cast<int64_t>(_file_size);
        }

        void flush() override {
            if (_is_open && _lfs_handle) {
                std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
                flush_internal();
            }
        }
    };

    lfsVFS::lfsVFS(
        std::shared_ptr<lfs_t> lfs_handle,
        std::shared_ptr<lfs_config_t> lfs_config,
        std::shared_ptr<VFSContext> lfs_context)
        : _lfs_handle(lfs_handle), _lfs_config(lfs_config), _lfs_context(lfs_context) {}

    lfsVFS::~lfsVFS() {
        if (_lfs_handle) {
            std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
            lfs_unmount(_lfs_handle.get());
        }
    }

    std::vector<Entry> lfsVFS::dir(const std::string& path) {
        auto res = listDir(path);
        if (res.has_value()) {
            return res.value();
        }
        return {};
    }

    Result<std::vector<Entry>> lfsVFS::listDir(const std::string& path) {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
        std::vector<Entry> entries;
        lfs_dir_t dir_handle;

        int err = lfs_dir_open(_lfs_handle.get(), &dir_handle, path.c_str());
        if (err != LFS_ERR_OK) {
            return Result<std::vector<Entry>>(lfsToErrorCode(err));
        }

        struct lfs_info info;
        while (true) {
            int res = lfs_dir_read(_lfs_handle.get(), &dir_handle, &info);
            if (res < 0) {
                break;
            }

            if (std::strcmp(info.name, ".") == 0 || std::strcmp(info.name, "..") == 0) {
                continue;
            }

            if (info.type == LFS_TYPE_REG) {
                entries.push_back(FileEntry(info.name, info.size));
            } else if (info.type == LFS_TYPE_DIR) {
                entries.push_back(DirectoryEntry(info.name, info.size));
            }
        }

        lfs_dir_close(_lfs_handle.get(), &dir_handle);
        return Result<std::vector<Entry>>(entries);
    }

    LegacyErrorCode lfsVFS::openFile(std::shared_ptr<IFileObject>& handle, const std::string& path, uint32_t flags) {
        auto res = open(path, flags);
        if (res.has_value()) {
            handle = res.value();
            return kCodeOK;
        }
        return lfsToLegacyErrorCode(static_cast<int>(res.error()));
    }

    Result<std::shared_ptr<IFileObject>> lfsVFS::open(const std::string& path, uint32_t flags) {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);

        uint32_t lfs_flags = 0;
        if (flags & kFileRead) { lfs_flags |= LFS_O_RDONLY; }
        if (flags & kFileWrite) { lfs_flags |= LFS_O_WRONLY; }
        if (flags & kFileCreateIfNotExists) { lfs_flags |= LFS_O_CREAT; }
        if (flags & kFileCreateFailIfExists) { lfs_flags |= LFS_O_EXCL; }
        if (flags & kFileTruncate) { lfs_flags |= LFS_O_TRUNC; }
        if (flags & kFileAppend) { lfs_flags |= LFS_O_APPEND; }

        auto file_obj = std::make_shared<VFSFileObject>(_lfs_handle, _lfs_context);

        int err = lfs_file_open(_lfs_handle.get(), &file_obj->_file_handle, path.c_str(), lfs_flags);
        if (err == LFS_ERR_NOENT && (flags & kFileCreateIfNotExists)) {
            // Automatically ensure parent directories exist
            size_t slash_pos = 0;
            while ((slash_pos = path.find('/', slash_pos + 1)) != std::string::npos) {
                std::string parent = path.substr(0, slash_pos);
                if (!parent.empty() && parent != "." && parent != "/") {
                    lfs_mkdir(_lfs_handle.get(), parent.c_str());
                }
            }
            err = lfs_file_open(_lfs_handle.get(), &file_obj->_file_handle, path.c_str(), lfs_flags);
        }

        if (err != LFS_ERR_OK) {
            return Result<std::shared_ptr<IFileObject>>(lfsToErrorCode(err));
        }

        file_obj->init_file_state();
        file_obj->_path = path;
        return Result<std::shared_ptr<IFileObject>>(std::static_pointer_cast<IFileObject>(file_obj));
    }

    LegacyErrorCode lfsVFS::existsFile(const std::string& path) {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
        struct lfs_info info;
        int err = lfs_stat(_lfs_handle.get(), path.c_str(), &info);
        return (err == LFS_ERR_OK) ? kCodeOK : kCodeFileNotFound;
    }

    LegacyErrorCode lfsVFS::deleteFile(const std::string& path) {
        auto res = remove(path);
        if (res.has_value()) return kCodeOK;
        return lfsToLegacyErrorCode(static_cast<int>(res.error()));
    }

    LegacyErrorCode lfsVFS::deleteDirectory(const std::string& path) {
        return deleteFile(path);
    }

    Result<void> lfsVFS::remove(const std::string& path) {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
        int err = lfs_remove(_lfs_handle.get(), path.c_str());
        if (err != LFS_ERR_OK) {
            return Result<void>(lfsToErrorCode(err));
        }
        return Result<void>::success();
    }

    Result<void> lfsVFS::rename(const std::string& oldpath, const std::string& newpath) {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
        int err = lfs_rename(_lfs_handle.get(), oldpath.c_str(), newpath.c_str());
        if (err != LFS_ERR_OK) {
            return Result<void>(lfsToErrorCode(err));
        }
        return Result<void>::success();
    }

    Result<lfs_fsinfo> lfsVFS::statFs() {
        std::lock_guard<std::recursive_mutex> lock(_lfs_context->mutex);
        lfs_fsinfo info;
        int err = lfs_fs_stat(_lfs_handle.get(), &info);
        if (err != LFS_ERR_OK) {
            return Result<lfs_fsinfo>(lfsToErrorCode(err));
        }
        return Result<lfs_fsinfo>(info);
    }

    // Generic VFS initializer for any IBlockDevice
    static Result<std::shared_ptr<IFileSystemDevice>> initVFSInternal(std::shared_ptr<IBlockDevice> device, bool format) {
        auto fs_handle = std::make_shared<lfs_t>();
        auto fs_config = std::make_shared<lfs_config_t>();
        auto fs_context = std::make_shared<lfsVFS::VFSContext>();

        fs_context->lfs_handle = fs_handle;
        fs_context->block_device = device;

        std::memset(fs_handle.get(), 0, sizeof(lfs_t));
        std::memset(fs_config.get(), 0, sizeof(lfs_config_t));

        fs_config->context = fs_context.get();
        fs_config->read = lfs_bd_cb_read;
        fs_config->write = lfs_bd_cb_prog;
        fs_config->erase = lfs_bd_cb_erase;
        fs_config->sync = lfs_bd_cb_sync;
        fs_config->allocate_block = lfs_bd_cb_alloc;
        fs_config->lock = lfs_bd_cb_lock;
        fs_config->unlock = lfs_bd_cb_unlock;

        fs_config->read_size = 1;
        fs_config->write_size = 1;
        fs_config->block_size = device->get_block_size();
        fs_config->block_count = device->get_block_count();
        fs_config->cache_size = fs_config->block_size;
        fs_config->erase_size = 0;
        fs_config->lookahead_size = fs_config->block_size;
        fs_config->block_cycles = -1;
        fs_config->file_max_size = 0x7FFFFFFFFFFFFFFFULL;
        fs_config->on_grow = false;

        if (format) {
            int err = lfs_format(fs_handle.get(), fs_config.get());
            if (err != LFS_ERR_OK) {
                return Result<std::shared_ptr<IFileSystemDevice>>(lfsToErrorCode(err));
            }
        }

        int err = lfs_mount(fs_handle.get(), fs_config.get());
        if (err != LFS_ERR_OK) {
            return Result<std::shared_ptr<IFileSystemDevice>>(lfsToErrorCode(err));
        }

        auto vfs = std::make_shared<lfsVFS>(fs_handle, fs_config, fs_context);
        return Result<std::shared_ptr<IFileSystemDevice>>(std::static_pointer_cast<IFileSystemDevice>(vfs));
    }

    Result<std::shared_ptr<IFileSystemDevice>> createVFSWithDevice(std::shared_ptr<IBlockDevice> device) {
        return initVFSInternal(device, true);
    }

    Result<std::shared_ptr<IFileSystemDevice>> openVFSWithDevice(std::shared_ptr<IBlockDevice> device) {
        return initVFSInternal(device, false);
    }

    // Legacy factory overloads
    static std::string wstringToString(const std::wstring& wstr) {
        std::string str;
        for (wchar_t wc : wstr) {
            str.push_back(static_cast<char>(wc));
        }
        return str;
    }

    LegacyErrorCode createVFS(const std::string& path, std::shared_ptr<IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {
        std::shared_ptr<IBlockDevice> device;
        if (backend == lfsVFS::Backend::kFileBackend) {
            device = std::make_shared<FileBlockDevice>(path, 64 * 1024, 8, true);
        } else {
            device = std::make_shared<MemoryBlockDevice>(64 * 1024, 8);
        }

        auto res = createVFSWithDevice(device);
        if (res.has_value()) {
            filesystem = res.value();
            return kCodeOK;
        }
        return lfsToLegacyErrorCode(static_cast<int>(res.error()));
    }

    LegacyErrorCode openVFS(const std::string& path, std::shared_ptr<IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {
        std::shared_ptr<IBlockDevice> device;
        if (backend == lfsVFS::Backend::kFileBackend) {
            device = std::make_shared<FileBlockDevice>(path, 64 * 1024, 8, false);
        } else {
            device = std::make_shared<MemoryBlockDevice>(64 * 1024, 8);
        }

        auto res = openVFSWithDevice(device);
        if (res.has_value()) {
            filesystem = res.value();
            return kCodeOK;
        }
        return lfsToLegacyErrorCode(static_cast<int>(res.error()));
    }

    LegacyErrorCode createVFS(const std::wstring& path, std::shared_ptr<IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {
        return createVFS(wstringToString(path), filesystem, backend);
    }

    LegacyErrorCode openVFS(const std::wstring& path, std::shared_ptr<IFileSystemDevice>& filesystem, lfsVFS::Backend backend) {
        return openVFS(wstringToString(path), filesystem, backend);
    }

} // namespace fs