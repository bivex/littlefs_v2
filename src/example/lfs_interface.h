#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include "lfs.h"
#include "lfs_result.h"
#include "lfs_block_device.h"

namespace fs {

    struct IFileObject;
    struct IFileSystemDevice;

    // Legacy enum for backward compatibility
    enum LegacyErrorCode {
        kCodeOK = 0,
        kCodeFileNotFound,
        kCodeObjectNotCompatible,
        kCodeBadDevice,
        kCodeNoDeviceSpace,
        kCodeUnknownError
    };

    enum FileOpenFlags : uint32_t {
        kFileRead = 1,
        kFileWrite = 2,
        kFileCreateIfNotExists = 0x0100,
        kFileCreateFailIfExists = 0x0200,
        kFileTruncate = 0x0400,
        kFileAppend = 0x0800,
    };

    struct Entry {
        enum Type {
            kEntryFile,
            kEntryDirectory
        };
    private:
        Type _type;
        std::string _path;
        uint64_t _size;
    protected:
        Entry(Type type, const std::string& path, uint64_t size)
            : _type(type)
            , _path(path)
            , _size(size) {}
    public:
        constexpr Type getType() const {
            return _type;
        }
        constexpr const std::string& getPath() const {
            return _path;
        }
        constexpr uint64_t getSize() const {
            return _size;
        }
    };

    struct FileEntry : public Entry {
    public:
        FileEntry(const std::string& path, uint64_t size)
            : Entry(kEntryFile, path, size) {}
    };

    struct DirectoryEntry : public Entry {
    public:
        DirectoryEntry(const std::string& path, uint64_t size)
            : Entry(kEntryDirectory, path, size) {}
    };

    struct IFileObject {
    public:
        enum SeekType {
            kSeekSet = 0,
            kSeekCur = 1,
            kSeekEnd = 2,
        };
        std::string _path;

    public:
        virtual ~IFileObject() = default;

        virtual int64_t read(void* data, uint64_t size) = 0;
        virtual int64_t write(const void* data, uint64_t size) = 0;

        virtual int64_t truncate(uint64_t size) = 0;
        virtual int64_t seek(uint64_t offset, SeekType type) = 0;
        virtual int64_t tell() = 0;
        virtual int64_t size() = 0;
        virtual void flush() = 0;
        virtual void close() = 0;
    };

    // RAII movable FileHandle wrapper
    class FileHandle {
    public:
        FileHandle() = default;
        explicit FileHandle(std::shared_ptr<IFileObject> file) : _file(std::move(file)) {}

        ~FileHandle() {
            if (_file) {
                _file->close();
            }
        }

        FileHandle(FileHandle&& other) noexcept = default;
        FileHandle& operator=(FileHandle&& other) noexcept = default;

        FileHandle(const FileHandle&) = delete;
        FileHandle& operator=(const FileHandle&) = delete;

        bool is_open() const noexcept { return _file != nullptr; }
        explicit operator bool() const noexcept { return is_open(); }

        IFileObject* get() const noexcept { return _file.get(); }
        IFileObject* operator->() const noexcept { return _file.get(); }
        IFileObject& operator*() const { return *_file; }
        std::shared_ptr<IFileObject> shared() const noexcept { return _file; }

    private:
        std::shared_ptr<IFileObject> _file;
    };

    struct IFileSystemDevice {
    public:
        virtual ~IFileSystemDevice() = default;

        virtual std::vector<Entry> dir(const std::string& path) = 0;
        virtual LegacyErrorCode openFile(std::shared_ptr<IFileObject>& handle, const std::string& path, uint32_t flags) = 0;
        virtual LegacyErrorCode existsFile(const std::string& path) = 0;
        virtual LegacyErrorCode deleteFile(const std::string& path) = 0;
        virtual LegacyErrorCode deleteDirectory(const std::string& path) = 0;

        // Modern Result-based interface
        virtual Result<std::vector<Entry>> listDir(const std::string& path) = 0;
        virtual Result<std::shared_ptr<IFileObject>> open(const std::string& path, uint32_t flags) = 0;
        virtual Result<void> remove(const std::string& path) = 0;
        virtual Result<void> rename(const std::string& oldpath, const std::string& newpath) = 0;
        virtual Result<lfs_fsinfo> statFs() = 0;
    };

    struct lfsVFS : public IFileSystemDevice {
        enum Backend {
            kMemoryBackend,
            kFileBackend
        };

        struct VFSContext {
            std::shared_ptr<lfs_t> lfs_handle;
            std::shared_ptr<IBlockDevice> block_device;
            std::recursive_mutex mutex;
            bool on_grow{false};
        };

    private:
        std::shared_ptr<lfs_t> _lfs_handle;
        std::shared_ptr<lfs_config_t> _lfs_config;
        std::shared_ptr<VFSContext> _lfs_context;

    public:
        lfsVFS(
            std::shared_ptr<lfs_t> lfs_handle,
            std::shared_ptr<lfs_config_t> lfs_config,
            std::shared_ptr<VFSContext> lfs_context
        );

        ~lfsVFS() override;

        std::vector<Entry> dir(const std::string& path) override;
        LegacyErrorCode openFile(std::shared_ptr<IFileObject>& handle, const std::string& path, uint32_t flags) override;
        LegacyErrorCode existsFile(const std::string& path) override;
        LegacyErrorCode deleteFile(const std::string& path) override;
        LegacyErrorCode deleteDirectory(const std::string& path) override;

        Result<std::vector<Entry>> listDir(const std::string& path) override;
        Result<std::shared_ptr<IFileObject>> open(const std::string& path, uint32_t flags) override;
        Result<void> remove(const std::string& path) override;
        Result<void> rename(const std::string& oldpath, const std::string& newpath) override;
        Result<lfs_fsinfo> statFs() override;

        std::recursive_mutex& getMutex() { return _lfs_context->mutex; }
    };

    // Observer Pattern: VFS Event Observer
    enum class VFSEventType {
        kFileOpened,
        kFileClosed,
        kFileWritten,
        kFileRemoved,
        kFileRenamed,
        kStorageGrown
    };

    class IVFSEventListener {
    public:
        virtual ~IVFSEventListener() = default;
        virtual void onVFSEvent(VFSEventType type, const std::string& path, uint64_t data_size) = 0;
    };

    // Adapter Pattern: std::streambuf C++ Stream Adapter for IFileObject
    class VFSStreamBuf : public std::streambuf {
    public:
        explicit VFSStreamBuf(std::shared_ptr<IFileObject> file, size_t buffer_size = 4096)
            : _file(std::move(file)), _buffer(buffer_size) {
            char* base = reinterpret_cast<char*>(_buffer.data());
            setg(base, base, base);
            setp(base, base + _buffer.size());
        }

        ~VFSStreamBuf() override {
            sync();
        }

    protected:
        int_type underflow() override {
            if (!_file) return traits_type::eof();
            int64_t bytes_read = _file->read(_buffer.data(), _buffer.size());
            if (bytes_read <= 0) return traits_type::eof();
            char* base = reinterpret_cast<char*>(_buffer.data());
            setg(base, base, base + bytes_read);
            return traits_type::to_int_type(*gptr());
        }

        int_type overflow(int_type ch) override {
            if (!_file) return traits_type::eof();
            if (sync() != 0) return traits_type::eof();
            if (!traits_type::eq_int_type(ch, traits_type::eof())) {
                char c = traits_type::to_char_type(ch);
                if (_file->write(&c, 1) != 1) return traits_type::eof();
            }
            return ch;
        }

        int sync() override {
            if (!_file) return 0;
            std::ptrdiff_t n = pptr() - pbase();
            if (n > 0) {
                if (_file->write(pbase(), static_cast<uint64_t>(n)) != n) return -1;
                char* base = reinterpret_cast<char*>(_buffer.data());
                setp(base, base + _buffer.size());
            }
            _file->flush();
            return 0;
        }

    private:
        std::shared_ptr<IFileObject> _file;
        std::vector<uint8_t> _buffer;
    };

    class VFSInputStream : public std::istream {
    public:
        explicit VFSInputStream(std::shared_ptr<IFileObject> file)
            : std::istream(&_sbuf), _sbuf(std::move(file)) {}
    private:
        VFSStreamBuf _sbuf;
    };

    class VFSOutputStream : public std::ostream {
    public:
        explicit VFSOutputStream(std::shared_ptr<IFileObject> file)
            : std::ostream(&_sbuf), _sbuf(std::move(file)) {}
    private:
        VFSStreamBuf _sbuf;
    };

    // Factory methods
    LegacyErrorCode openVFS(
        const std::wstring& path,
        std::shared_ptr<IFileSystemDevice>& filesystem,
        lfsVFS::Backend backend = lfsVFS::Backend::kMemoryBackend);

    LegacyErrorCode createVFS(
        const std::wstring& path,
        std::shared_ptr<IFileSystemDevice>& filesystem,
        lfsVFS::Backend backend = lfsVFS::Backend::kMemoryBackend);

    LegacyErrorCode openVFS(
        const std::string& path,
        std::shared_ptr<IFileSystemDevice>& filesystem,
        lfsVFS::Backend backend = lfsVFS::Backend::kMemoryBackend);

    LegacyErrorCode createVFS(
        const std::string& path,
        std::shared_ptr<IFileSystemDevice>& filesystem,
        lfsVFS::Backend backend = lfsVFS::Backend::kMemoryBackend);

    Result<std::shared_ptr<IFileSystemDevice>> createVFSWithDevice(std::shared_ptr<IBlockDevice> device);
    Result<std::shared_ptr<IFileSystemDevice>> openVFSWithDevice(std::shared_ptr<IBlockDevice> device);

    // Builder Pattern (GoF Builder Pattern)
    class VFSBuilder {
    public:
        VFSBuilder() = default;

        VFSBuilder& withMemoryBackend(lfs_size_t block_size = 64 * 1024, lfs_size_t block_count = 8) {
            _device = BlockDeviceFactory::createMemory(block_size, block_count);
            return *this;
        }

        VFSBuilder& withFileBackend(const std::string& path, lfs_size_t block_size = 64 * 1024, lfs_size_t block_count = 8, bool create_new = false) {
            _device = BlockDeviceFactory::createFile(path, block_size, block_count, create_new);
            return *this;
        }

        VFSBuilder& withDevice(std::shared_ptr<IBlockDevice> device) {
            _device = std::move(device);
            return *this;
        }

        VFSBuilder& withCrypto(uint64_t key = 0xA5A55A5AA5A55A5AULL) {
            if (_device) _device = BlockDeviceFactory::wrapCrypto(_device, key);
            return *this;
        }

        VFSBuilder& withFaultInjection() {
            if (_device) _device = BlockDeviceFactory::wrapFaultInjection(_device);
            return *this;
        }

        VFSBuilder& withStatistics() {
            if (_device) _device = BlockDeviceFactory::wrapStatistics(_device);
            return *this;
        }

        Result<std::shared_ptr<IFileSystemDevice>> buildCreate() {
            if (!_device) return ErrorCode::kInvalidParameter;
            return createVFSWithDevice(_device);
        }

        Result<std::shared_ptr<IFileSystemDevice>> buildOpen() {
            if (!_device) return ErrorCode::kInvalidParameter;
            return openVFSWithDevice(_device);
        }

    private:
        std::shared_ptr<IBlockDevice> _device;
    };

} // namespace fs