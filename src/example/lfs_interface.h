#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>
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

} // namespace fs