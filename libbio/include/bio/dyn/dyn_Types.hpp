
#pragma once
#include <bio/crypto/crypto_Sha256.hpp>

namespace bio::dyn {

    namespace nro {

        struct HeaderStart {
            u32 unk;
            u32 module_header_offset;
            u8 pad[8];
        };

        struct Header {
            HeaderStart start;
            u32 magic;
            u32 unk;
            u32 size;
            u32 unk2;
            u64 segments[3];
            u32 bss_size;
            u32 unk3;
            u8 build_id[0x20];
            u32 pad[0x20];
        };

        constexpr u32 NRO0 = 0x304F524E;

        inline u32 GetNROSize(void *nro_buf) {
            RET_UNLESS(nro_buf != nullptr, 0);
            return reinterpret_cast<Header*>(nro_buf)->size;
        }

    }

    namespace nrr {

        struct Header {
            u32 magic;
            u32 unk_0;
            u8 reserved[8];
            u8 certification[0x220];
            u8 sig[0x100];
            u64 program_id;
            u32 size;
            u8 kind;
            u8 reserved2[3];
            u32 hash_offset;
            u32 hash_count;
            u8 reserved3[8];
        };

        constexpr u32 NRR0 = 0x3052524E;

        inline constexpr u32 GetNroOffset(u32 nro_index) {
            return sizeof(Header) + crypto::Sha256Context::HashSize * nro_index;
        }
        
        inline constexpr u32 GetNrrSize(u32 nro_count) {
            return GetNroOffset(nro_count);
        }

        inline void InitializeHeader(void *nrr_buf, u64 nrr_size, u64 program_id, u32 nro_count) {
            auto header = reinterpret_cast<Header*>(nrr_buf);
            __builtin_memset(header, 0, sizeof(Header));
            header->magic = NRR0;
            header->program_id = program_id;
            header->size = nrr_size;
            header->hash_offset = sizeof(Header);
            header->hash_count = nro_count;
        }

        inline void SetNROHashAt(void *nrr_buf, const void *nro_buf, u64 nro_size, u32 nro_index) {
            const auto offset = GetNroOffset(nro_index);
            auto buf = reinterpret_cast<void*>(reinterpret_cast<u8*>(nrr_buf) + offset);
            crypto::CalculateSha256(nro_buf, nro_size, buf);
        }

    }

    namespace mod {

        struct Header {
            u32 magic;
            u32 dynamic;
            u32 bss_start;
            u32 bss_end;
            u32 unwind_start;
            u32 unwind_end;
            u32 module_object;
        };

        constexpr u32 MOD0 = 0x30444F4D;

    }

}