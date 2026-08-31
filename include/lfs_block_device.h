#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <cstdio>
#include <cstring>
#include <atomic>
#include "lfs.h"

namespace fs {

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

    // In-memory block device implementation
    class MemoryBlockDevice : public IBlockDevice {
    public:
        MemoryBlockDevice(lfs_size_t block_size = 64 * 1024, lfs_size_t block_count = 8)
            : _block_size(block_size), _block_count(block_count) {
            _storage.resize(static_cast<size_t>(_block_size * _block_count), 0xFF);
        }

        int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) override {
            if (block >= _block_count || off + size > _block_size) {
                return LFS_ERR_IO;
            }
            size_t src_offset = static_cast<size_t>(block * _block_size + off);
            std::memcpy(buffer, &_storage[src_offset], static_cast<size_t>(size));
            return LFS_ERR_OK;
        }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            if (block >= _block_count || off + size > _block_size) {
                return LFS_ERR_IO;
            }
            size_t dst_offset = static_cast<size_t>(block * _block_size + off);
            std::memcpy(&_storage[dst_offset], buffer, static_cast<size_t>(size));
            return LFS_ERR_OK;
        }

        int erase(lfs_block_t block) override {
            if (block >= _block_count) {
                return LFS_ERR_IO;
            }
            size_t dst_offset = static_cast<size_t>(block * _block_size);
            std::memset(&_storage[dst_offset], 0xFF, static_cast<size_t>(_block_size));
            return LFS_ERR_OK;
        }

        int sync() override {
            return LFS_ERR_OK;
        }

        int allocate_block() override {
            lfs_size_t new_count = _block_count + 8;
            _storage.resize(static_cast<size_t>(_block_size * new_count), 0xFF);
            _block_count = new_count;
            return LFS_ERR_OK;
        }

        lfs_size_t get_block_size() const override { return _block_size; }
        lfs_size_t get_block_count() const override { return _block_count; }
        void set_block_count(lfs_size_t count) override {
            _block_count = count;
            _storage.resize(static_cast<size_t>(_block_size * _block_count), 0xFF);
        }

        const std::vector<uint8_t>& get_storage() const { return _storage; }

    private:
        lfs_size_t _block_size;
        lfs_size_t _block_count;
        std::vector<uint8_t> _storage;
    };

    // Host-file backed block device implementation
    class FileBlockDevice : public IBlockDevice {
    public:
        FileBlockDevice(const std::string& filepath, lfs_size_t block_size = 64 * 1024, lfs_size_t initial_blocks = 8, bool truncate_new = false)
            : _filepath(filepath), _block_size(block_size), _block_count(initial_blocks), _file(nullptr) {
            
            const char* mode = truncate_new ? "w+b" : "r+b";
            _file = std::fopen(filepath.c_str(), mode);
            if (!_file && !truncate_new) {
                // Try create if opening for read/write failed
                _file = std::fopen(filepath.c_str(), "w+b");
            }
            if (_file && truncate_new) {
                grow_file_to_blocks(_block_count);
            } else if (_file) {
                // Determine existing blocks
                lfs_fseek64(_file, 0, SEEK_END);
                int64_t fsize = lfs_ftell64(_file);
                if (fsize > 0 && fsize >= static_cast<int64_t>(_block_size)) {
                    _block_count = static_cast<lfs_size_t>(fsize / _block_size);
                } else {
                    grow_file_to_blocks(_block_count);
                }
            }
        }

        ~FileBlockDevice() override {
            if (_file) {
                std::fflush(_file);
                std::fclose(_file);
                _file = nullptr;
            }
        }

        bool is_open() const { return _file != nullptr; }

        int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) override {
            if (!_file || block >= _block_count || off + size > _block_size) {
                return LFS_ERR_IO;
            }
            int64_t target_pos = static_cast<int64_t>(block * _block_size + off);
            if (lfs_fseek64(_file, target_pos, SEEK_SET) != 0) {
                return LFS_ERR_IO;
            }
            size_t bytes_read = std::fread(buffer, 1, static_cast<size_t>(size), _file);
            return (bytes_read == static_cast<size_t>(size)) ? LFS_ERR_OK : LFS_ERR_IO;
        }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            if (!_file || block >= _block_count || off + size > _block_size) {
                return LFS_ERR_IO;
            }
            int64_t target_pos = static_cast<int64_t>(block * _block_size + off);
            if (lfs_fseek64(_file, target_pos, SEEK_SET) != 0) {
                return LFS_ERR_IO;
            }
            size_t bytes_written = std::fwrite(buffer, 1, static_cast<size_t>(size), _file);
            return (bytes_written == static_cast<size_t>(size)) ? LFS_ERR_OK : LFS_ERR_IO;
        }

        int erase(lfs_block_t block) override {
            (void)block;
            return LFS_ERR_OK;
        }

        int sync() override {
            if (!_file) return LFS_ERR_IO;
            return (std::fflush(_file) == 0) ? LFS_ERR_OK : LFS_ERR_IO;
        }

        int allocate_block() override {
            lfs_size_t new_count = _block_count + 8;
            if (grow_file_to_blocks(new_count) == 0) {
                _block_count = new_count;
                return LFS_ERR_OK;
            }
            return LFS_ERR_NOSPC;
        }

        lfs_size_t get_block_size() const override { return _block_size; }
        lfs_size_t get_block_count() const override { return _block_count; }
        void set_block_count(lfs_size_t count) override {
            _block_count = count;
            grow_file_to_blocks(count);
        }

    private:
        static inline int lfs_fseek64(FILE* stream, int64_t offset, int whence) {
#if defined(_WIN32)
            return _fseeki64(stream, offset, whence);
#else
            return fseeko(stream, static_cast<off_t>(offset), whence);
#endif
        }

        static inline int64_t lfs_ftell64(FILE* stream) {
#if defined(_WIN32)
            return _ftelli64(stream);
#else
            return ftello(stream);
#endif
        }

        int grow_file_to_blocks(lfs_size_t blocks) {
            if (!_file) return -1;
            int64_t target_size = static_cast<int64_t>(blocks * _block_size);
            if (lfs_fseek64(_file, target_size - 1, SEEK_SET) != 0) {
                return -1;
            }
            uint8_t zero = 0;
            if (std::fwrite(&zero, 1, 1, _file) != 1) {
                return -1;
            }
            std::fflush(_file);
            return 0;
        }

        std::string _filepath;
        lfs_size_t _block_size;
        lfs_size_t _block_count;
        FILE* _file;
    };

    // Base Decorator (GoF Decorator Pattern)
    class BlockDeviceDecorator : public IBlockDevice {
    public:
        explicit BlockDeviceDecorator(std::shared_ptr<IBlockDevice> underlying)
            : _underlying(std::move(underlying)) {}

        int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) override {
            return _underlying->read(block, off, buffer, size);
        }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            return _underlying->write(block, off, buffer, size);
        }

        int erase(lfs_block_t block) override {
            return _underlying->erase(block);
        }

        int sync() override {
            return _underlying->sync();
        }

        int allocate_block() override {
            return _underlying->allocate_block();
        }

        lfs_size_t get_block_size() const override { return _underlying->get_block_size(); }
        lfs_size_t get_block_count() const override { return _underlying->get_block_count(); }
        void set_block_count(lfs_size_t count) override { _underlying->set_block_count(count); }

        std::shared_ptr<IBlockDevice> get_underlying() const noexcept { return _underlying; }

    protected:
        std::shared_ptr<IBlockDevice> _underlying;
    };

    // Decorator: Cryptographic block transformation (XOR / Scrambler per-block)
    class CryptoBlockDevice : public BlockDeviceDecorator {
    public:
        CryptoBlockDevice(std::shared_ptr<IBlockDevice> underlying, uint64_t key = 0xA5A55A5AA5A55A5AULL)
            : BlockDeviceDecorator(std::move(underlying)), _key(key) {}

        int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) override {
            int res = _underlying->read(block, off, buffer, size);
            if (res == LFS_ERR_OK) {
                transform(block, off, static_cast<uint8_t*>(buffer), size);
            }
            return res;
        }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            std::vector<uint8_t> encrypted(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + size);
            transform(block, off, encrypted.data(), size);
            return _underlying->write(block, off, encrypted.data(), size);
        }

    private:
        void transform(lfs_block_t block, lfs_off_t off, uint8_t* data, lfs_size_t size) {
            uint64_t block_key = _key ^ (block * 0x9E3779B97F4A7C15ULL);
            for (lfs_size_t i = 0; i < size; ++i) {
                lfs_off_t pos = off + i;
                uint8_t key_byte = static_cast<uint8_t>((block_key >> ((pos % 8) * 8)) & 0xFF);
                uint8_t mask = key_byte ^ static_cast<uint8_t>((pos * 37) & 0xFF);
                data[i] ^= mask;
            }
        }

        uint64_t _key;
    };

    // Decorator: Telemetry & I/O Metrics monitoring
    class StatisticsBlockDevice : public BlockDeviceDecorator {
    public:
        struct Stats {
            std::atomic<uint64_t> reads_count{0};
            std::atomic<uint64_t> writes_count{0};
            std::atomic<uint64_t> erases_count{0};
            std::atomic<uint64_t> syncs_count{0};
            std::atomic<uint64_t> bytes_read{0};
            std::atomic<uint64_t> bytes_written{0};
        };

        explicit StatisticsBlockDevice(std::shared_ptr<IBlockDevice> underlying)
            : BlockDeviceDecorator(std::move(underlying)) {}

        int read(lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) override {
            _stats.reads_count++;
            _stats.bytes_read += size;
            return _underlying->read(block, off, buffer, size);
        }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            _stats.writes_count++;
            _stats.bytes_written += size;
            return _underlying->write(block, off, buffer, size);
        }

        int erase(lfs_block_t block) override {
            _stats.erases_count++;
            return _underlying->erase(block);
        }

        int sync() override {
            _stats.syncs_count++;
            return _underlying->sync();
        }

        const Stats& get_stats() const noexcept { return _stats; }
        void reset_stats() noexcept {
            _stats.reads_count = 0;
            _stats.writes_count = 0;
            _stats.erases_count = 0;
            _stats.syncs_count = 0;
            _stats.bytes_read = 0;
            _stats.bytes_written = 0;
        }

    private:
        Stats _stats;
    };

    // Decorator: Fault-Injection device for Chaos / Power-Loss testing
    class FaultInjectBlockDevice : public BlockDeviceDecorator {
    public:
        explicit FaultInjectBlockDevice(std::shared_ptr<IBlockDevice> underlying)
            : BlockDeviceDecorator(std::move(underlying)), _fail_after_writes(-1), _write_count(0), _corrupt_next_write(false) {}

        void set_fail_after_writes(int count) {
            _fail_after_writes = count;
            _write_count = 0;
        }

        void set_corrupt_next_write(bool corrupt) {
            _corrupt_next_write = corrupt;
        }

        int get_write_count() const { return _write_count.load(); }

        int write(lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) override {
            if (_fail_after_writes >= 0 && _write_count >= _fail_after_writes) {
                return LFS_ERR_IO; // Simulating sudden power cut / disk error
            }
            _write_count++;

            if (_corrupt_next_write) {
                _corrupt_next_write = false;
                std::vector<uint8_t> corrupted(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + size);
                if (!corrupted.empty()) {
                    corrupted[0] ^= 0xFF; // Invert first byte to simulate write corruption
                }
                return _underlying->write(block, off, corrupted.data(), size);
            }

            return _underlying->write(block, off, buffer, size);
        }

        int sync() override {
            if (_fail_after_writes >= 0 && _write_count >= _fail_after_writes) {
                return LFS_ERR_IO;
            }
            return _underlying->sync();
        }

    private:
        int _fail_after_writes;
        std::atomic<int> _write_count;
        bool _corrupt_next_write;
    };

    // Factory (GoF Factory Pattern)
    class BlockDeviceFactory {
    public:
        static std::shared_ptr<IBlockDevice> createMemory(lfs_size_t block_size = 64 * 1024, lfs_size_t block_count = 8) {
            return std::make_shared<MemoryBlockDevice>(block_size, block_count);
        }

        static std::shared_ptr<IBlockDevice> createFile(const std::string& path, lfs_size_t block_size = 64 * 1024, lfs_size_t block_count = 8, bool create_new = false) {
            return std::make_shared<FileBlockDevice>(path, block_size, block_count, create_new);
        }

        static std::shared_ptr<IBlockDevice> wrapCrypto(std::shared_ptr<IBlockDevice> dev, uint64_t key) {
            return std::make_shared<CryptoBlockDevice>(std::move(dev), key);
        }

        static std::shared_ptr<IBlockDevice> wrapFaultInjection(std::shared_ptr<IBlockDevice> dev) {
            return std::make_shared<FaultInjectBlockDevice>(std::move(dev));
        }

        static std::shared_ptr<IBlockDevice> wrapStatistics(std::shared_ptr<IBlockDevice> dev) {
            return std::make_shared<StatisticsBlockDevice>(std::move(dev));
        }
    };

} // namespace fs
