
#pragma once
#include <bio/base.hpp>

// Note: most of this is a C++, no-std port of elf.h C header

namespace bio::dyn {

    namespace elf {

        enum class Tag : i64 {
            RelaOffset = 7,
            RelaSize = 8,
            RelaEntrySize = 9,
            RelaCount = 0x6FFFFFF9,
        };

        struct Dyn {
            i64 tag;
            u64 val_ptr;

            inline constexpr bool FindValue(elf::Tag tag, u64 &out_value) {
                u64 *found = nullptr;
                auto dynamic = this;
                for(; dynamic->tag != 0; dynamic++) {
                    if(dynamic->tag == static_cast<i64>(tag)) {
                        if(found != nullptr) {
                            return false;
                        }
                        else {
                            found = &dynamic->val_ptr;
                        }
                    }
                }
                if(found == nullptr) {
                    return false;
                }
                out_value = *found;
                return true;
            }

            inline bool FindOffset(elf::Tag tag, void *&out_value, void *aslr_base) {
                u64 intermediate = 0;
                auto res = this->FindValue(tag, intermediate);
                if(!res) {
                    return false;
                }
                out_value = reinterpret_cast<void*>(reinterpret_cast<u8*>(aslr_base) + intermediate);
                return true;
            }
        
        };

        union Info {
            u64 value;
            struct {
                u32 reloc_type;
                u32 symbol;
            };
        };

        struct Rel {
            u64 offset;
            Info info;
        };

        struct Rela {
            u64 offset;
            Info info;
            i64 addend;
        };

        enum class RelocationType : u32 {
            AArch64Relative = 1027,
        };

    }

}