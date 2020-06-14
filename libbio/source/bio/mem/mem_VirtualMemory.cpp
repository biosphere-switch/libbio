#include <bio/mem/mem_VirtualMemory.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/mem/mem_Utils.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/os/os_Mutex.hpp>

namespace bio::mem {

    VirtualRegion g_AddressSpace;
    VirtualRegion g_StackRegion;
    VirtualRegion g_HeapRegion;
    VirtualRegion g_LegacyAliasRegion;

    namespace {

        os::Mutex g_VirtualMemoryLock;
        u64 g_CurrentAddress = 0;

    }

    Result AllocateVirtual(u64 size, void *&out_addr) {
        BIO_RET_UNLESS(IsAligned(size, PageAlignment), result::ResultInvalidSize);
        os::ScopedMutexLock lk(g_VirtualMemoryLock);
        
        svc::MemoryInfo info;
        u32 page_info;
        auto addr = g_CurrentAddress;
        while(true) {
            addr += PageAlignment;
            
            if(!g_AddressSpace.In(addr)) {
                addr = g_AddressSpace.start;
            }

            auto cur_addr = addr + size;
            BIO_RES_TRY(svc::QueryMemory(info, page_info, addr));
            auto info_addr = info.address + info.size;
            if(info.type != 0) {
                addr = info_addr;
                continue;
            }

            if(cur_addr > info_addr) {
                addr = info_addr;
                continue;
            }

            auto end = cur_addr - 1;

            #define _BIO_MEM_IS_IN_VIRTUAL_REGION(region) \
            if(region.In(addr) || region.In(end)) { \
                addr = region.end; \
                continue; \
            }

            _BIO_MEM_IS_IN_VIRTUAL_REGION(g_StackRegion)
            _BIO_MEM_IS_IN_VIRTUAL_REGION(g_HeapRegion)
            _BIO_MEM_IS_IN_VIRTUAL_REGION(g_LegacyAliasRegion)

            #undef _BIO_MEM_IS_IN_VIRTUAL_REGION

            break;
        }

        g_CurrentAddress = addr + size;
        out_addr = reinterpret_cast<void*>(addr);
        return ResultSuccess;
    }

}