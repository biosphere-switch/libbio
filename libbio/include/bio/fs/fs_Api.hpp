
#pragma once
#include <bio/fs/fs_File.hpp>
#include <bio/fs/fs_Directory.hpp>
#include <bio/fs/fs_Results.hpp>

namespace bio::fs {

    Result MountDevice(const char *name, mem::SharedObject<service::fsp::FileSystem> fs);
    Result MountSdCard(const char *name);
    void UnmountDevice(const char *name);

    Result CreateFile(const char *path, service::fsp::FileAttribute attr, u64 size);
    Result OpenFile(const char *path, service::fsp::FileOpenMode mode, mem::SharedObject<File> &out_file);
    Result DeleteFile(const char *path);

    Result CreateDirectory(const char *path);
    Result OpenDirectory(const char *path, service::fsp::DirectoryOpenMode mode, mem::SharedObject<Directory> &out_directory);
    Result DeleteDirectory(const char *path);
    Result DeleteDirectoryRecursively(const char *path);

    Result GetEntryType(const char *path, service::fsp::DirectoryEntryType &out_type);

}