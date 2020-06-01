
#pragma once
#include <bio/base.hpp>

namespace bio::service::fsp {

    enum class FileOpenMode : u32 {
        Read = BIO_BITMASK(0),
        Write = BIO_BITMASK(1),
        Append = BIO_BITMASK(2),
    };

    inline constexpr FileOpenMode operator|(FileOpenMode lhs, FileOpenMode rhs) {
        return static_cast<FileOpenMode>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
    }

    inline constexpr FileOpenMode operator&(FileOpenMode lhs, FileOpenMode rhs) {
        return static_cast<FileOpenMode>(static_cast<u32>(lhs) & static_cast<u32>(rhs));
    }

    constexpr u32 MaxPathLength = 0x301;

    enum class FileCreateOption : u32 {
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

}