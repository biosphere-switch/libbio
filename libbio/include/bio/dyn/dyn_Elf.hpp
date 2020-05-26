
#pragma once
#include <bio/dyn/dyn_Results.hpp>

// Note: most of this is a C++, no-std port of elf.h C header

namespace bio::dyn {

    namespace elf {

        enum class Tag : i64 {
            Needed = 1,
            PltRelSize = 2,
            Hash = 4,
            StrTab = 5,
            SymTab = 6,
            RelaOffset = 7,
            RelaSize = 8,
            RelaEntrySize = 9,
            SymEnt = 11,
            RelOffset = 17,
            RelSize = 18,
            RelEntrySize = 19,
            PltRel = 20,
            JmpRel = 23,
            InitArray = 25,
            FiniArray = 26,
            InitArraySize = 27,
            FiniArraySize = 28,
            RelaCount = 0x6FFFFFF9,
        };

        struct Dyn {
            i64 tag;
            u64 val_ptr;

            inline constexpr Result FindValue(elf::Tag tag, u64 &out_value) {
                u64 *found = nullptr;
                auto dynamic = this;
                for(; dynamic->tag != 0; dynamic++) {
                    if(dynamic->tag == static_cast<i64>(tag)) {
                        if(found != nullptr) {
                            return result::ResultDuplicatedDtEntry;
                        }
                        else {
                            found = &dynamic->val_ptr;
                        }
                    }
                }
                if(found == nullptr) {
                    return result::ResultMissingDtEntry;
                }
                out_value = *found;
                return ResultSuccess;
            }

            inline Result FindOffset(elf::Tag tag, void *&out_value, void *aslr_base) {
                u64 intermediate = 0;
                BIO_RES_TRY(this->FindValue(tag, intermediate));

                out_value = reinterpret_cast<void*>(reinterpret_cast<u8*>(aslr_base) + intermediate);
                return ResultSuccess;
            }
        
        };

        struct Sym {
            u32 name;
            u8 info;
            u8 other;
            u16 shndx;
            u64 value;
            u64 size;
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
            AArch64Abs64 = 257,
            AArch64GlobDat = 1025,
            AArch64JumpSlot = 1026,
            AArch64Relative = 1027,
        };

        inline constexpr u64 HashString(const char *name) {
            u64 h = 0;
            u64 g = 0;
            while(*name) {
                h = (h << 4) + static_cast<u8>(*name++);
                if((g = (h & 0xF0000000)) != 0) {
                    h ^= g >> 24;
                }
                h &= ~g;
            }
            return h;
        }

    }

}