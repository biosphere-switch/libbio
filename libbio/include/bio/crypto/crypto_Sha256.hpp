
#pragma once
#include <bio/mem/mem_Memory.hpp>

namespace bio::crypto {

    // Grabbed from libnx, hardware-accelerated impl

    class Sha256Context {

        public:
            static constexpr u64 HashSize = 0x20;
            static constexpr u64 HashSize32 = HashSize / sizeof(u32);
            static constexpr u64 BlockSize = 0x40;

            static constexpr u32 InitialHash[HashSize32] = {
                0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
            };

        private:
            u32 intermediate_hash[HashSize32];
            u8 block[BlockSize];
            u64 bits_consumed;
            u64 num_buffered;
            bool finalized;

            void EnsureFinalized();
            void ProcessBlocks(const u8 *buf, u64 num_blocks);

        public:
            Sha256Context() : intermediate_hash(), block(), bits_consumed(0), num_buffered(0), finalized(false) {
                mem::Copy(this->intermediate_hash, InitialHash, HashSize);
            }

            void Update(const void *buf, u64 size);

            inline void GetHash(void *out_dest) {
                this->EnsureFinalized();
                auto dest32 = reinterpret_cast<u32*>(out_dest);
                for(u32 i = 0; i < HashSize32; i++) {
                    dest32[i] = __builtin_bswap32(this->intermediate_hash[i]);
                }
            }

    };

    inline void CalculateSha256(const void *buf, u64 size, void *out) {
        Sha256Context ctx;
        ctx.Update(buf, size);
        ctx.GetHash(out);
    }

}