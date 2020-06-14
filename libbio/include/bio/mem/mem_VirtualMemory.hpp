
#pragma once
#include <bio/base.hpp>

namespace bio::mem {

    struct VirtualRegion {
        u64 start;
        u64 end;

        inline constexpr bool In(u64 address) {
            return (address >= this->start) && (address < this->end);
        }

    };

    enum class VirtualRegionType {
        Stack,
        Heap,
        LegacyAlias,
    };

    constexpr u32 VirtualRegionCount = 3;

    Result AllocateVirtual(u64 size, void *&out_addr);

    template<typename T>
    Result AllocateVirtual(T *&out_addr) {
        return AllocateVirtual(sizeof(T), reinterpret_cast<void*&>(out_addr));
    }

}