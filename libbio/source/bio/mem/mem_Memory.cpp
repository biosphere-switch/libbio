#include <bio/mem/mem_Memory.hpp>
#include <bio/svc/svc_Impl.hpp>

namespace bio::mem {

    // By default, take 4 pages of space.

    __attribute__((weak))
    u64 g_AllocationTableSize = 0x4000;

    namespace {

        u64 g_BaseAddress = 0;
        u64 g_TotalSize = 0;

        inline u64 GetActualBaseAddress() {
            return g_BaseAddress + g_AllocationTableSize;
        }

        inline constexpr u64 ComputePadding(u64 base_address, u64 alignment) {
            const auto multiplier = (base_address / alignment) + 1;
            const auto aligned_address = multiplier * alignment;
            return aligned_address - base_address;
        }

        inline u64 GetAllocationTableCount() {
            return g_AllocationTableSize / sizeof(AllocationInfo);
        }

        inline AllocationInfo *GetAllocationTableAddress() {
            return reinterpret_cast<AllocationInfo*>(g_BaseAddress);
        }

        inline AllocationInfo *FindEmptyAllocationInfo() {
            auto alloc_table_addr = GetAllocationTableAddress();
            for(u64 i = 0; i < GetAllocationTableCount(); i++) {
                auto alloc_info = &alloc_table_addr[i];
                // Check if the allocation info is being used.
                if(!alloc_info->IsValid()) {
                    return alloc_info;
                }
            }
            return nullptr;
        }

        inline bool IsAddressAllocated(u64 addr, AllocationInfo *&out_info) {
            auto alloc_table_addr = GetAllocationTableAddress();
            for(u64 i = 0; i < GetAllocationTableCount(); i++) {
                auto alloc_info = &alloc_table_addr[i];
                // Check if any region is inside this address + size.
                if(alloc_info->IsValid()) {
                    if(alloc_info->IsAddressIn(addr)) {
                        out_info = alloc_info;
                        return true;
                    }
                }
            }
            return false;
        }
        
        inline bool IsRegionAllocated(u64 addr, u64 size, AllocationInfo *&out_info) {
            auto alloc_table_addr = GetAllocationTableAddress();
            for(u64 i = 0; i < GetAllocationTableCount(); i++) {
                auto alloc_info = &alloc_table_addr[i];
                // Check if any region is inside this address + size.
                if(alloc_info->IsValid()) {
                    if(alloc_info->IsInRegion(addr, size)) {
                        out_info = alloc_info;
                        return true;
                    }
                }
            }
            return false;
        }

    }

    void Initialize(void *address, u64 size) {
        g_BaseAddress = reinterpret_cast<u64>(address);
        g_TotalSize = size;
        ZeroCount(GetAllocationTableAddress(), GetAllocationTableCount());
    }

    Result AllocateAligned(u64 alignment, u64 size, void *&out_addr) {
        BIO_RET_UNLESS(size > 0, result::ResultInvalidSize);

        // Keep iterating through memory.
        auto base_addr = GetActualBaseAddress();

        auto empty_alloc_info = FindEmptyAllocationInfo();
        BIO_RET_UNLESS(empty_alloc_info != nullptr, result::ResultOutOfAllocationTableSpace);

        const auto needs_align = alignment != 0;
        while(true) {
            AllocationInfo *alloc_info = nullptr;
            if(!IsRegionAllocated(base_addr, size, alloc_info)) {
                // If we need alignment, try to align this address.
                if(needs_align && (base_addr % alignment) != 0) {
                    auto addr_pad = ComputePadding(base_addr, alignment);
                    base_addr += addr_pad;
                }
                // Check if the memory fits in the space.
                auto base_addr_end = base_addr + size;
                AllocationInfo *tmp_info = nullptr;
                if(!IsRegionAllocated(base_addr_end, size, tmp_info)) {
                    // Also check that we don't go out of memory
                    if(base_addr_end < (g_BaseAddress + g_TotalSize)) {
                        // We found a suitable space.
                        // Create an allocation info entry, and return the address.
                        empty_alloc_info->address = base_addr;
                        empty_alloc_info->size = size;
                        out_addr = reinterpret_cast<void*>(base_addr);
                        return ResultSuccess;
                    }
                }
            }
            else {
                // Continue with the search.
                base_addr = alloc_info->GetEndAddress();
            }
        }

        return result::ResultOutOfMemory;
    }

    void Free(void *ptr) {
        auto alloc_table_addr = GetAllocationTableAddress();
        for(u64 i = 0; i < GetAllocationTableCount(); i++) {
            auto alloc_info = &alloc_table_addr[i];
            // Find the address's allocation info, and clear it.
            if(alloc_info->address == reinterpret_cast<u64>(ptr)) {
                alloc_info->Clear();
            }
        }
    }

    bool IsAllocated(void *address) {
        AllocationInfo *info = nullptr;
        return IsAddressAllocated(reinterpret_cast<u64>(address), info);
    }

	bool IsFree(void *address) {
        return !IsAllocated(address);
    }

	AllocationInfo GetAllocationInfo(void *address) {
        AllocationInfo *info = nullptr;
        if(IsAddressAllocated(reinterpret_cast<u64>(address), info)) {
            return *info;
        }
        return mem::Zeroed<AllocationInfo>();
    }

}
