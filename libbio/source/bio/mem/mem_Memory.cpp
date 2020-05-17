#include <bio/mem/mem_Memory.hpp>
#include <bio/svc/svc_Impl.hpp>

namespace bio::mem {

    namespace {

        u64 g_BaseAddress = 0;
        u64 g_TotalSize = 0;
        u64 g_CurrentOffset = 0;

        inline constexpr u64 ComputePadding(u64 base_address, u64 alignment) {
            const auto multiplier = (base_address / alignment) + 1;
            const auto aligned_address = multiplier * alignment;
            return aligned_address - base_address;
        }

    }

    void Initialize(void *address, u64 size) {
        g_BaseAddress = reinterpret_cast<u64>(address);
        g_TotalSize = size;
        g_CurrentOffset = 0;
    }

    void *AllocateAligned(u64 alignment, u64 size) {
        const auto cur_address = g_BaseAddress + g_CurrentOffset;
        
        u64 padding = 0;
        if((alignment != 0) && ((g_CurrentOffset % alignment) != 0)) {
            padding = ComputePadding(cur_address, alignment);
        }

        if((g_CurrentOffset + padding + size) > g_TotalSize) {
            // Out of memory
            return nullptr;
        }

        g_CurrentOffset += padding + size;
        return reinterpret_cast<void*>(cur_address + padding);
    }

    void Free(void *ptr) {
        // This system can't free memory :P
    }

}
