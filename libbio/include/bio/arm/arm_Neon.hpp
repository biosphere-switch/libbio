
#pragma once
#include <bio/base.hpp>

namespace bio::arm {

    // Small implementation of arm_neon.h, only stuff used by hw-accelerated crypto

    #define _BIO_ARM_NEON_V_TYPE_DEF(t, n) using t ## x ## n = __attribute__((neon_vector_type(n))) t

    _BIO_ARM_NEON_V_TYPE_DEF(i8, 16);
    _BIO_ARM_NEON_V_TYPE_DEF(u32, 4);

    #undef _BIO_ARM_NEON_V_TYPE_DEF

    inline u32x4 vld1q_u32(const u32 *ptr) {
        return __builtin_neon_vld1q_v(ptr, 50);
    }

    inline constexpr u32x4 vdupq_n_u32(u32 p) {
        return (u32x4) { p, p, p, p };
    }

    inline u32x4 vaddq_u32(u32x4 a, u32x4 b) {
        return (u32x4)(a + b);
    }

    inline void vst1q_u32(u32 *a, u32x4 b) {
        __builtin_neon_vst1q_v(a, static_cast<i8x16>(b), 50);
    }

}