
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
    Result SetMemoryPermission(void *addr, u64 size, u32 perm); // 0x02
    Result QueryMemory(MemoryInfo &out_info, u32 &out_page_info, u64 address); // 0x06
    void __attribute__((noreturn)) ExitProcess(); // 0x07
    Result CreateThread(Handle &out_handle, void *entry, void *entry_arg, void *stack_top, i32 priority, i32 cpu_id); // 0x08
    Result StartThread(Handle handle); // 0x09
    void __attribute__((noreturn)) ExitThread(); // 0x0A
    void SleepThread(i64 nano); // 0x0B
    Result GetThreadPriority(i32 &out_priority, Handle handle); // 0x0C
    Result CloseHandle(Handle handle); // 0x16
    Result ArbitrateLock(u32 wait_tag, u32 *tag_location, u32 self_tag); // 0x1A
    Result ArbitrateUnlock(u32 *tag_location); // 0x1B
    Result ConnectToNamedPort(Handle &out_handle, const char *name); // 0x1F
    Result SendSyncRequest(Handle handle); // 0x21
    Result GetProcessId(u64 &out_process_id, Handle handle); // 0x24
    Result GetThreadId(u64 &out_thread_id, Handle handle); // 0x25
    Result OutputDebugString(const char *str, u64 len); // 0x27
    Result GetInfo(u64 &out_info, u32 id_0, Handle handle, u64 id_1); // 0x29

}

#include <bio/util/util_String.hpp>

#define DEBUG_LOG(msg) ::bio::svc::OutputDebugString(msg, BIO_UTIL_STRLEN(msg))
#define DEBUG_LOG_FMT(fmt, ...) ({ \
    char msg[0x400]; \
    ::bio::mem::ZeroArray(msg); \
    const auto len = static_cast<u32>(::bio::util::SPrintf(msg, fmt, ##__VA_ARGS__)); \
    ::bio::svc::OutputDebugString(msg, len); \
})
