
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

            inline void EnsureFinalized() {
                if(!this->finalized) {
                    this->bits_consumed += 8 * this->num_buffered;
                    this->block[this->num_buffered++] = 0x80;

                    constexpr u64 last_block_max_size = BlockSize - sizeof(u64);
                    if(this->num_buffered <= last_block_max_size) {
                        mem::Fill(this->block + this->num_buffered, 0, last_block_max_size - this->num_buffered);
                    }
                    else {
                        mem::Fill(this->block + this->num_buffered, 0, BlockSize - this->num_buffered);
                        this->ProcessBlocks(this->block, 1);

                        mem::Fill(this->block, 0, last_block_max_size);
                    }

                    auto big_endian_bits_consumed = __builtin_bswap64(this->bits_consumed);
                    mem::Copy(this->block + last_block_max_size, &big_endian_bits_consumed, sizeof(big_endian_bits_consumed));
                    this->ProcessBlocks(this->block, 1);
                    this->finalized = true;
                }
            }

            void ProcessBlocks(const u8 *buf, u64 num_blocks);

        public:
            Sha256Context() : intermediate_hash(), block(), bits_consumed(0), num_buffered(0), finalized(false) {
                mem::Copy(this->intermediate_hash, InitialHash, HashSize);
            }

            inline void GetHash(void *out_dest) {
                this->EnsureFinalized();
                auto dest32 = reinterpret_cast<u32*>(out_dest);
                for(u32 i = 0; i < HashSize32; i++) {
                    dest32[i] = __builtin_bswap32(this->intermediate_hash[i]);
                }
            }

            inline void Update(const void *buf, u64 size) {
                const u8 *cur_src = reinterpret_cast<const u8*>(buf);
                this->bits_consumed += (((this->num_buffered + size) / BlockSize) * BlockSize) * 8;

                if(this->num_buffered > 0) {
                    const u64 needed = BlockSize - this->num_buffered;
                    const u64 copyable = (size > needed ? needed : size);
                    mem::Copy(&this->block[this->num_buffered], cur_src, copyable);
                    cur_src += copyable;
                    this->num_buffered += copyable;
                    size -= copyable;

                    if (this->num_buffered == BlockSize) {
                        this->ProcessBlocks(this->block, 1);
                        this->num_buffered = 0;
                    }
                }

                if(size >= BlockSize) {
                    const u64 num_blocks = size / BlockSize;
                    this->ProcessBlocks(cur_src, num_blocks);
                    size -= BlockSize * num_blocks;
                    cur_src += BlockSize * num_blocks;
                }

                if(size > 0) {
                    mem::Copy(this->block, cur_src, size);
                    this->num_buffered = size;
                }
            }

    };

    inline void CalculateSha256(const void *buf, u64 size, void *out) {
        Sha256Context ctx;
        ctx.Update(buf, size);
        ctx.GetHash(out);
    }

}