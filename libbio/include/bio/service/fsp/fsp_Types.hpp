
#pragma once
#include <bio/base.hpp>

namespace bio::service::fsp {

    enum class FileOpenMode : u32 {
        Read = BIO_BITMASK(0),
        Write = BIO_BITMASK(1),
        Append = BIO_BITMASK(2),
    };

    BIO_ENUM_BIT_OPERATORS(FileOpenMode, u32)

    constexpr u32 MaxPathLength = 0x301;

    enum class FileAttribute : u8 {
        None = 0,
        ConcatenationFile = BIO_BITMASK(0),
    };

    enum class FileReadOption : u32 {
        None = 0,
    };

    enum class FileWriteOption : u32 {
        None = 0,
        Flush = BIO_BITMASK(0),
    };

    enum class DirectoryEntryType : u8 {
        Directory,
        File,
    };

    struct DirectoryEntry {
        char name[MaxPathLength];
        FileAttribute file_attr;
        u8 pad[2];
        DirectoryEntryType type;
        u8 pad2[3];
        u64 file_size;
    };
    static_assert(sizeof(DirectoryEntry) == 0x310);

    enum class DirectoryOpenMode : i64 {
        ReadDirectories = BIO_BITMASK(0),
        ReadFiles = BIO_BITMASK(1),
        NoFileSizes = BIO_BITMASK(31),
    };

    BIO_ENUM_BIT_OPERATORS(DirectoryOpenMode, i64)

}