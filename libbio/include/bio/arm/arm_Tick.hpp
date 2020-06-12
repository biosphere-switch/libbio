
#pragma once
#include <bio/base.hpp>

namespace bio::arm {

    inline u64 GetSystemTick() {
        u64 ret;
        __asm__ __volatile__ ("mrs %x[data], cntpct_el0" : [data] "=r" (ret));
        return ret;
    }

    inline u64 GetSystemTickFrequency() {
        u64 ret;
        __asm__ ("mrs %x[data], cntfrq_el0" : [data] "=r" (ret));
        return ret;
    }

    inline constexpr u64 ConvertToTicks(u64 nanoseconds) {
        return (nanoseconds * 12) / 625;
    }

    inline constexpr u64 ConvertToNanoseconds(u64 ticks) {
        return (ticks * 625) / 12;
    }

}