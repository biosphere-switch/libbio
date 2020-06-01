
#pragma once
#include <bio/fs/fs_File.hpp>
#include <bio/fs/fs_Results.hpp>

namespace bio::fs {

    Result MountFileSystem(const char *name, mem::SharedObject<service::fsp::FileSystem> fs);
    Result MountSdCard(const char *name);

    Result CreateFile(const char *path, service::fsp::FileCreateOption option, u64 size);
    Result OpenFile(const char *path, service::fsp::FileOpenMode mode, mem::SharedObject<File> &out_file);

}