#include <bio/crypto/crypto_Sha256.hpp>
#include <bio/arm/arm_Neon.hpp>

namespace bio::crypto {

    namespace {

        alignas(Sha256Context::BlockSize) constexpr u32 g_RoundConstants[0x40] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

    }

    void Sha256Context::ProcessBlocks(const u8 *buf, u64 num_blocks) {
        arm::u32x4 prev_hash0 = arm::vld1q_u32(this->intermediate_hash + 0);
        arm::u32x4 prev_hash1 = arm::vld1q_u32(this->intermediate_hash + 4);
        arm::u32x4 cur_hash0  = arm::vdupq_n_u32(0);
        arm::u32x4 cur_hash1  = arm::vdupq_n_u32(0);

        while(num_blocks > 0) {
            arm::u32x4 round_constant0, round_constant1;
            arm::u32x4 data0, data1, data2, data3;
            arm::u32x4 tmp0, tmp1, tmp2, tmp3;
            arm::u32x4 tmp_hash;

            __asm__ __volatile__ (
                "ldp       %q[data0], %q[data1], [%[buf]], #0x20\n"
                "ldp       %q[data2], %q[data3], [%[buf]], #0x20\n"
                "add       %[cur_hash0].4s, %[cur_hash0].4s, %[prev_hash0].4s\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0x00]\n"
                "add       %[cur_hash1].4s, %[cur_hash1].4s, %[prev_hash1].4s\n"
                "rev32     %[data0].16b, %[data0].16b\n"
                "rev32     %[data1].16b, %[data1].16b\n"
                "rev32     %[data2].16b, %[data2].16b\n"
                "rev32     %[data3].16b, %[data3].16b\n"
                "add       %[tmp0].4s, %[data0].4s, %[round_constant0].4s\n"
                "add       %[tmp1].4s, %[data1].4s, %[round_constant1].4s\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0x20]\n"
                "sha256su0 %[data0].4s, %[data1].4s\n"
                "mov       %[prev_hash0].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp0].4s\n"
                "mov       %[prev_hash1].16b, %[cur_hash1].16b\n"
                "sha256h2  %q[cur_hash1], %q[prev_hash0], %[tmp0].4s\n"
                "sha256su0 %[data1].4s, %[data2].4s\n"
                "sha256su1 %[data0].4s, %[data2].4s, %[data3].4s\n"
                "add       %[tmp2].4s, %[data2].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp1].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp1].4s\n"
                "sha256su0 %[data2].4s, %[data3].4s\n"
                "sha256su1 %[data1].4s, %[data3].4s, %[data0].4s\n"
                "add       %[tmp3].4s, %[data3].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0x40]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp2].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp2].4s\n"
                "sha256su0 %[data3].4s, %[data0].4s\n"
                "sha256su1 %[data2].4s, %[data0].4s, %[data1].4s\n"
                "add       %[tmp0].4s, %[data0].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp3].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp3].4s\n"
                "sha256su0 %[data0].4s, %[data1].4s\n"
                "sha256su1 %[data3].4s, %[data1].4s, %[data2].4s\n"
                "add       %[tmp1].4s, %[data1].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0x60]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp0].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp0].4s\n"
                "sha256su0 %[data1].4s, %[data2].4s\n"
                "sha256su1 %[data0].4s, %[data2].4s, %[data3].4s\n"
                "add       %[tmp2].4s, %[data2].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp1].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp1].4s\n"
                "sha256su0 %[data2].4s, %[data3].4s\n"
                "sha256su1 %[data1].4s, %[data3].4s, %[data0].4s\n"
                "add       %[tmp3].4s, %[data3].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0x80]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp2].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp2].4s\n"
                "sha256su0 %[data3].4s, %[data0].4s\n"
                "sha256su1 %[data2].4s, %[data0].4s, %[data1].4s\n"
                "add       %[tmp0].4s, %[data0].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp3].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp3].4s\n"
                "sha256su0 %[data0].4s, %[data1].4s\n"
                "sha256su1 %[data3].4s, %[data1].4s, %[data2].4s\n"
                "add       %[tmp1].4s, %[data1].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0xA0]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp0].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp0].4s\n"
                "sha256su0 %[data1].4s, %[data2].4s\n"
                "sha256su1 %[data0].4s, %[data2].4s, %[data3].4s\n"
                "add       %[tmp2].4s, %[data2].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp1].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp1].4s\n"
                "sha256su0 %[data2].4s, %[data3].4s\n"
                "sha256su1 %[data1].4s, %[data3].4s, %[data0].4s\n"
                "add       %[tmp3].4s, %[data3].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0xC0]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp2].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp2].4s\n"
                "sha256su0 %[data3].4s, %[data0].4s\n"
                "sha256su1 %[data2].4s, %[data0].4s, %[data1].4s\n"
                "add       %[tmp0].4s, %[data0].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp3].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp3].4s\n"
                "sha256su1 %[data3].4s, %[data1].4s, %[data2].4s\n"
                "add       %[tmp1].4s, %[data1].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "ldp       %q[round_constant0], %q[round_constant1], [%[round_constants], 0xE0]\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp0].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp0].4s\n"
                "add       %[tmp2].4s, %[data2].4s, %[round_constant0].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp1].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp1].4s\n"
                "add       %[tmp3].4s, %[data3].4s, %[round_constant1].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp2].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp2].4s\n"
                "mov       %[tmp_hash].16b, %[cur_hash0].16b\n"
                "sha256h   %q[cur_hash0], %q[cur_hash1], %[tmp3].4s\n"
                "sha256h2  %q[cur_hash1], %q[tmp_hash], %[tmp3].4s\n"
                : [data0]"=w"(data0), [data1]"=w"(data1), [data2]"=w"(data2), [data3]"=w"(data3),
                [tmp0]"=w"(tmp0), [tmp1]"=w"(tmp1), [tmp2]"=w"(tmp2), [tmp3]"=w"(tmp3),
                [round_constant0]"=w"(round_constant0), [round_constant1]"=w"(round_constant1),
                [cur_hash0]"+w"(cur_hash0), [cur_hash1]"+w"(cur_hash1),
                [prev_hash0]"+w"(prev_hash0), [prev_hash1]"+w"(prev_hash1),
                [tmp_hash]"=w"(tmp_hash), [buf]"+r"(buf)
                : [round_constants]"r"(g_RoundConstants)
                :
            );

            num_blocks--;
        }

        cur_hash0 = arm::vaddq_u32(prev_hash0, cur_hash0);
        cur_hash1 = arm::vaddq_u32(prev_hash1, cur_hash1);
        arm::vst1q_u32(this->intermediate_hash + 0, cur_hash0);
        arm::vst1q_u32(this->intermediate_hash + 4, cur_hash1);
    }

}