# littlefs_v2 C++ API Reference

Comprehensive reference for `littlefs_v2` Virtual File System interfaces, data types, RAII wrappers, and block device decorators.

---

## Table of Contents
1. [Error Handling & Result<T, E>](#1-error-handling--resultt-e)
2. [VFS Interfaces (IFileSystemDevice & IFileObject)](#2-vfs-interfaces)
3. [RAII FileHandle Wrapper](#3-raii-filehandle-wrapper)
4. [Block Device Abstraction (IBlockDevice)](#4-block-device-abstraction)
5. [Factory & Lifecycle Functions](#5-factory--lifecycle-functions)

---

## 1. Error Handling & Result<T, E>

Defined in [`include/lfs_result.h`](file:///Volumes/External/Code/littlefs_v2/include/lfs_result.h).

### `enum class fs::ErrorCode`
Enumerates standardized error codes across the VFS layer:

| Code | Value | Description |
| :--- | :---: | :--- |
| `kSuccess` | `0` | Operation succeeded |
| `kFileNotFound` | `1` | File or directory does not exist (`LFS_ERR_NOENT`) |
| `kFileExists` | `2` | Entry already exists (`LFS_ERR_EXIST`) |
| `kNotDirectory` | `3` | Expected directory, but found regular file (`LFS_ERR_NOTDIR`) |
| `kIsDirectory` | `4` | Expected regular file, but found directory (`LFS_ERR_ISDIR`) |
| `kDirectoryNotEmpty` | `5` | Directory cannot be deleted because it contains entries (`LFS_ERR_NOTEMPTY`) |
| `kBadFileDescriptor` | `6` | Attempted operation on an unopened or invalid handle (`LFS_ERR_BADF`) |
| `kFileTooLarge` | `7` | Target file exceeded maximum supported size (`LFS_ERR_FBIG`) |
| `kInvalidParameter` | `8` | Invalid arguments or flags (`LFS_ERR_INVAL`) |
| `kNoSpaceOnDevice` | `9` | Storage capacity exhausted and auto-grow failed (`LFS_ERR_NOSPC`) |
| `kNoMemory` | `10` | Out of host memory (`LFS_ERR_NOMEM`) |
| `kCorrupted` | `13` | Filesystem metadata corruption detected (`LFS_ERR_CORRUPT`) |
| `kIoError` | `14` | Low-level storage hardware read/write failure (`LFS_ERR_IO`) |

### `template <typename T, typename E = ErrorCode> class fs::Result`
Modern return type for all VFS operations:

```cpp
Result<std::shared_ptr<IFileObject>> res = vfs->open("config.json", fs::kFileRead);
if (res.has_value()) {
    auto file = res.value();
    // use file...
} else {
    std::cerr << "Error: " << fs::errorToString(res.error()) << "\n";
}
```

#### Key Methods:
* `bool has_value() const`: Returns `true` if operation succeeded.
* `explicit operator bool() const`: Evaluates to `true` on success.
* `T& value()` / `const T& value()`: Accesses the contained value (throws or asserts if error).
* `E error() const`: Returns the error code.
* `T* operator->()` / `T& operator*()`: Dereferences the value directly.

---

## 2. VFS Interfaces

Defined in [`src/example/lfs_interface.h`](file:///Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h).

### `enum fs::FileOpenFlags`
Bitmask flags used when opening files:

```cpp
enum FileOpenFlags : uint32_t {
    kFileRead               = 0x0001, // Open for reading
    kFileWrite              = 0x0002, // Open for writing
    kFileCreateIfNotExists  = 0x0100, // Create file if it does not exist
    kFileCreateFailIfExists  = 0x0200, // Fail if file already exists (exclusive)
    kFileTruncate           = 0x0400, // Truncate file length to 0
    kFileAppend             = 0x0800  // Seek to end before each write
};
```

---

### `struct fs::IFileObject`
Abstract handle to an open file within the VFS. Fully thread-safe.

```cpp
struct IFileObject {
    enum SeekType {
        kSeekSet = 0, // Seek from beginning of file
        kSeekCur = 1, // Seek relative to current position
        kSeekEnd = 2  // Seek relative to end of file
    };

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
```

---

### `struct fs::IFileSystemDevice`
Abstract interface representing a mounted Virtual File System instance.

```cpp
struct IFileSystemDevice {
    virtual ~IFileSystemDevice() = default;

    // Modern Result-based API
    virtual Result<std::shared_ptr<IFileObject>> open(const std::string& path, uint32_t flags) = 0;
    virtual Result<std::vector<Entry>> listDir(const std::string& path) = 0;
    virtual Result<void> remove(const std::string& path) = 0;
    virtual Result<void> rename(const std::string& oldpath, const std::string& newpath) = 0;
    virtual Result<lfs_fsinfo> statFs() = 0;

    // Legacy backward-compatible API
    virtual LegacyErrorCode openFile(std::shared_ptr<IFileObject>& handle, const std::string& path, uint32_t flags) = 0;
    virtual std::vector<Entry> dir(const std::string& path) = 0;
    virtual LegacyErrorCode existsFile(const std::string& path) = 0;
    virtual LegacyErrorCode deleteFile(const std::string& path) = 0;
};
```

---

## 3. RAII FileHandle Wrapper

`fs::FileHandle` is a move-only smart wrapper around `std::shared_ptr<IFileObject>`. It guarantees that `flush()` and `close()` are executed deterministically when the handle leaves scope:

```cpp
{
    auto res = vfs->open("data.bin", fs::kFileWrite | fs::kFileCreateIfNotExists);
    if (!res) return;

    fs::FileHandle file(res.value());
    file->write("payload", 7);
    // Automatic flush() and close() occur here upon scope exit
}
```

---

## 4. Block Device Abstraction (IBlockDevice)

Defined in [`include/lfs_block_device.h`](file:///Volumes/External/Code/littlefs_v2/include/lfs_block_device.h).

```cpp
class IBlockDevice {
public:
    virtual ~IBlockDevice() = default;
    virtual int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) = 0;
    virtual int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) = 0;
    virtual int erase(lfs_block_t block) = 0;
    virtual int sync() = 0;
    virtual int allocate_block() { return LFS_ERR_NOSPC; }
    virtual lfs_size_t get_block_size() const = 0;
    virtual lfs_size_t get_block_count() const = 0;
    virtual void set_block_count(lfs_size_t count) = 0;
};
```

### Standard Implementations & Decorators:

1. **`fs::MemoryBlockDevice`**:
   * Stores blocks in `std::vector<uint8_t>`.
   * Ultra-fast (up to 16.8 GB/s read, 3.7 GB/s write).

2. **`fs::FileBlockDevice`**:
   * Uses host-OS 64-bit file streams (`fseeko` / `_fseeki64`).
   * Supports persistent disk containers (`.vfs` / `.bin`).

3. **`fs::CryptoBlockDevice` (Decorator)**:
   * Wraps any `IBlockDevice` and applies per-block cryptographic keystream scrambling on `write()` and `read()`.

4. **`fs::FaultInjectBlockDevice` (Decorator)**:
   * Intercepts I/O calls to simulate write failure after $N$ operations or inject bit corruptions for chaos testing.

---

## 5. Factory & Lifecycle Functions

```cpp
namespace fs {
    // Generic device factory
    Result<std::shared_ptr<IFileSystemDevice>> createVFSWithDevice(std::shared_ptr<IBlockDevice> device);
    Result<std::shared_ptr<IFileSystemDevice>> openVFSWithDevice(std::shared_ptr<IBlockDevice> device);

    // Convenience helpers
    LegacyErrorCode createVFS(const std::string& path, std::shared_ptr<IFileSystemDevice>& vfs, lfsVFS::Backend backend);
    LegacyErrorCode openVFS(const std::string& path, std::shared_ptr<IFileSystemDevice>& vfs, lfsVFS::Backend backend);
}
```
