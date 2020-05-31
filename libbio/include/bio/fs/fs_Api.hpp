
#pragma once
#include <bio/fs/fs_Types.hpp>
#include <bio/fs/fs_Results.hpp>

namespace bio::fs {

    Result MountFileSystem(const char *name, mem::SharedObject<service::fsp::FileSystem> fs);
    Result MountSdCard(const char *name);

    Result CreateFile(const char *path, u32 flags, u64 size);

}