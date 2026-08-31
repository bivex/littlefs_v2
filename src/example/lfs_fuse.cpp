#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>

#include "lfs_interface.h"

static std::shared_ptr<fs::IFileSystemDevice> g_vfs;
static std::mutex g_vfs_mutex;

static std::string normalize_path(const char* path) {
    if (!path) return "";
    if (path[0] == '/') return std::string(path + 1);
    return std::string(path);
}

static int lfs_fuse_getattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));
    std::string rel_path = normalize_path(path);

    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    if (rel_path.empty() || rel_path == ".") {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, fs::kFileRead) == fs::kCodeOK) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = file->size();
        stbuf->st_blocks = (stbuf->st_size + 512 - 1) / 512;
        return 0;
    }

    auto entries = g_vfs->dir("/");
    for (const auto& entry : entries) {
        if (entry.getPath() == rel_path || entry.getPath() == "/" + rel_path) {
            if (entry.getType() == fs::Entry::kEntryDirectory) {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
            } else {
                stbuf->st_mode = S_IFREG | 0644;
                stbuf->st_nlink = 1;
                stbuf->st_size = entry.getSize();
                stbuf->st_blocks = (stbuf->st_size + 512 - 1) / 512;
            }
            return 0;
        }
    }

    return -ENOENT;
}

static int lfs_fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                            off_t offset, struct fuse_file_info* fi,
                            enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    filler(buf, ".", NULL, 0, (enum fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (enum fuse_fill_dir_flags)0);

    std::lock_guard<std::mutex> lock(g_vfs_mutex);
    std::string rel_path = normalize_path(path);
    auto entries = g_vfs->dir(rel_path.empty() ? "/" : rel_path);

    for (const auto& entry : entries) {
        const std::string& entry_name = entry.getPath();
        filler(buf, entry_name.c_str(), NULL, 0, (enum fuse_fill_dir_flags)0);
    }

    return 0;
}

static int lfs_fuse_open(const char* path, struct fuse_file_info* fi) {
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    uint32_t flags = fs::kFileRead;
    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        flags |= fs::kFileWrite;
    }

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, flags) != fs::kCodeOK) {
        return -ENOENT;
    }

    return 0;
}

static int lfs_fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    (void) mode;
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    std::shared_ptr<fs::IFileObject> file;
    uint32_t flags = fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists;
    if (g_vfs->openFile(file, rel_path, flags) != fs::kCodeOK) {
        return -EIO;
    }

    return 0;
}

static int lfs_fuse_read(const char* path, char* buf, size_t size, off_t offset,
                         struct fuse_file_info* fi) {
    (void) fi;
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, fs::kFileRead) != fs::kCodeOK) {
        return -ENOENT;
    }

    file->seek(offset, fs::IFileObject::kSeekSet);
    int64_t bytes_read = file->read(buf, size);
    return (bytes_read < 0) ? -EIO : (int)bytes_read;
}

static int lfs_fuse_write(const char* path, const char* buf, size_t size,
                          off_t offset, struct fuse_file_info* fi) {
    (void) fi;
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, fs::kFileRead | fs::kFileWrite | fs::kFileCreateIfNotExists) != fs::kCodeOK) {
        return -ENOENT;
    }

    file->seek(offset, fs::IFileObject::kSeekSet);
    int64_t bytes_written = file->write(buf, size);
    file->flush();

    return (bytes_written < 0) ? -EIO : (int)bytes_written;
}

static int lfs_fuse_unlink(const char* path) {
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    if (g_vfs->deleteFile(rel_path) != fs::kCodeOK) {
        return -ENOENT;
    }
    return 0;
}

static int lfs_fuse_truncate(const char* path, off_t size, struct fuse_file_info* fi) {
    (void) fi;
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, fs::kFileRead | fs::kFileWrite) != fs::kCodeOK) {
        return -ENOENT;
    }

    if (file->truncate(size) != fs::kCodeOK) {
        return -EIO;
    }
    file->flush();
    return 0;
}

static int lfs_fuse_fsync(const char* path, int isdatasync, struct fuse_file_info* fi) {
    (void) isdatasync;
    (void) fi;
    std::string rel_path = normalize_path(path);
    std::lock_guard<std::mutex> lock(g_vfs_mutex);

    std::shared_ptr<fs::IFileObject> file;
    if (g_vfs->openFile(file, rel_path, fs::kFileRead | fs::kFileWrite) == fs::kCodeOK) {
        file->flush();
    }
    return 0;
}

static struct fuse_operations lfs_oper = {
    .getattr  = lfs_fuse_getattr,
    .unlink   = lfs_fuse_unlink,
    .truncate = lfs_fuse_truncate,
    .open     = lfs_fuse_open,
    .read     = lfs_fuse_read,
    .write    = lfs_fuse_write,
    .fsync    = lfs_fuse_fsync,
    .readdir  = lfs_fuse_readdir,
    .create   = lfs_fuse_create,
};

int main(int argc, char *argv[]) {
    std::cout << "Starting littlefs_v2 FUSE Driver..." << std::endl;

    std::wstring fs_img = L"/tmp/littlefs_v2.vfs";
    if (fs::openVFS(fs_img, g_vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
        if (fs::createVFS(fs_img, g_vfs, fs::lfsVFS::Backend::kFileBackend) != fs::kCodeOK) {
            std::cerr << "Failed to initialize VFS backend!" << std::endl;
            return 1;
        }
    }

    std::cout << "littlefs_v2 VFS backend initialized successfully." << std::endl;
    return fuse_main(argc, argv, &lfs_oper, NULL);
}
