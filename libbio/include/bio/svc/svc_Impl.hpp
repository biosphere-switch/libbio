
#pragma once
#include <bio/base.hpp>

namespace bio::svc {

    using Handle = u32;
    constexpr Handle CurrentProcessPseudoHandle = 0xFFFF8001;

    struct MemoryInfo {
        u64 address;
        u64 size;
        u32 type;
        u32 attr;
        u32 perm;
        u32 device_refcount;
        u32 ipc_refcount;
        u32 pad;
    };

    Result SetHeapSize(void *&out_addr, u64 size); // 0x01
    Result QueryMemory(MemoryInfo &out_info, u32 &out_page_info, u64 address); // 0x06
    void __attribute__((noreturn)) ExitProcess(); // 0x07
    Result CloseHandle(Handle handle); // 0x16
    Result ConnectToNamedPort(Handle &out_handle, const char *name); // 0x1F
    Result SendSyncRequest(Handle handle); // 0x21
    Result OutputDebugString(const char *str, u64 len); // 0x27
    Result GetInfo(u64 &out_info, u32 id_0, Handle handle, u64 id_1); // 0x29

}

#define DEBUG_LOG(msg) svc::OutputDebugString(msg, __builtin_strlen(msg))
#define DEBUG_PTR(ptr) svc::OutputDebugString(reinterpret_cast<const char*>(ptr), 0)
#define DEBUG_NUM(num) svc::OutputDebugString(reinterpret_cast<const char*>(static_cast<u64>(num)), 0)

#define DEBUG_LOG_PTR(msg, ptr) ({ \
    DEBUG_LOG(msg); \
    DEBUG_PTR(ptr); \
})

#define DEBUG_LOG_NUM(msg, num) ({ \
    DEBUG_LOG(msg); \
    DEBUG_NUM(num); \
})
