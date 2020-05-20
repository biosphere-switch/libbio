
#pragma once
#include <bio/dyn/dyn_Types.hpp>
#include <bio/dyn/dyn_Elf.hpp>

namespace bio::dyn {

    using InitFiniArrayFunction = void(*)();
    using InitFiniArray = InitFiniArrayFunction*;
    using InitArray = InitFiniArray;
    using FiniArray = InitFiniArray;

    inline void RelocateModuleBase(void *base, elf::Dyn *dyn) {
        auto base8 = reinterpret_cast<u8*>(base);
        u64 rela_off = 0;
        auto rc = dyn->FindValue(elf::Tag::RelaOffset, rela_off);
        if(rc.IsSuccess()) {
            u64 rela_size = 0;
            rc = dyn->FindValue(elf::Tag::RelaSize, rela_size);
            if(rc.IsSuccess()) {
                u64 rela_ent = 0;
                rc = dyn->FindValue(elf::Tag::RelaEntrySize, rela_ent);
                if(rc.IsSuccess()) {
                    u64 rela_count = 0;
                    rc = dyn->FindValue(elf::Tag::RelaCount, rela_count);
                    if(rc.IsSuccess()) {
                        if(rela_size == (rela_ent * rela_count)) {
                            auto rela_base = reinterpret_cast<elf::Rela*>(base8 + rela_off);
                            for(u64 i = 0; i < rela_count; i++) {
                                auto rela = rela_base[i];
                                switch(static_cast<elf::RelocationType>(rela.info.reloc_type)) {
                                    case elf::RelocationType::AArch64Relative: {
                                        if(rela.info.symbol == 0) {
                                            auto offset = reinterpret_cast<void**>(base8 + rela.offset);
                                            *offset = reinterpret_cast<void*>(base8 + rela.addend);
                                        }
                                        break;
                                    }
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    inline void RelocateModule(void *base) {
        auto base8 = reinterpret_cast<u8*>(base);
        auto start = reinterpret_cast<nro::HeaderStart*>(base);
        auto mod_header = reinterpret_cast<mod::Header*>(base8 + start->module_header_offset);
        if(mod_header->magic == mod::MOD0) {
            auto dyn = reinterpret_cast<elf::Dyn*>(reinterpret_cast<u8*>(mod_header) + mod_header->dynamic);
            RelocateModuleBase(base, dyn);
        }
    }

}