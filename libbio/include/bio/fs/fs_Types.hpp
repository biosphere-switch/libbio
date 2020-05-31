
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/service/fsp/fsp_FileSystemService.hpp>

namespace bio::fs {

    enum class PathNameType : u8 {
        Invalid,
        Root,
        Normal,
    };

    struct PathName {

        static constexpr u32 MaxLength = 0xFF;

        char name[MaxLength];
        PathNameType type;

        inline bool Equals(const PathName &name) {
            if(this->type == name.type) {
                if(util::Strcmp(this->name, name.name) == 0) {
                    return true;
                }
            }
            return false;
        }

        inline void SetName(const char *name) {
            util::Strncpy(this->name, name, MaxLength);
        }

    };
    static_assert(sizeof(PathName) == 0x100);

    constexpr char PathSeparator = '/';
    constexpr char RootTerminator = ':';
    constexpr char Nul = '\0';

    struct Device {
        PathName root_name;
        mem::SharedObject<service::fsp::FileSystem> fs;
    };

}