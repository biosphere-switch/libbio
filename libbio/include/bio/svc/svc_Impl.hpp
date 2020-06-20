
#pragma once
#include <bio/base.hpp>

namespace bio::svc {

    using Handle = u32;

    constexpr Handle CurrentThreadPseudoHandle = 0xFFFF8000;
    constexpr Handle CurrentProcessPseudoHandle = 0xFFFF8001;

    constexpr i64 IndefiniteWait = 0xFFFF'FFFF'FFFF'FFFF;
    constexpr i64 YieldWithoutCoreMigration = 0;
    constexpr i64 YieldWithCoreMigration = -1;
    constexpr i64 YieldToAnyThread = -2;

    using ThreadEntrypointFunction = void(*)(void*);

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

    enum class MemoryPermission : u32 {
        None = 0,
        Read = BIO_BITMASK(0),
        Write = BIO_BITMASK(1),
        Execute = BIO_BITMASK(2),
        DontCare = BIO_BITMASK(28),
    };

    BIO_ENUM_BIT_OPERATORS(MemoryPermission, u32)

    enum class MemoryAttribute : u32 {
        None = 0,
        Borrowed = BIO_BITMASK(0),
        IpcMapped = BIO_BITMASK(1),
        DeviceMapped = BIO_BITMASK(2),
        Uncached = BIO_BITMASK(3),
    };

    BIO_ENUM_BIT_OPERATORS(MemoryAttribute, u32)

    enum class InfoId : u32 {
        CoreMask = 0,
        PriorityMask = 1,
        AliasRegionAddress = 2,
        AliasRegionSize = 3,
        HeapRegionAddress = 4,
        HeapRegionSize = 5,
        TotalMemorySize = 6,
        UsedMemorySize = 7,
        DebuggerAttached = 8,
        ResourceLimit = 9,
        IdleTickCount = 10,
        RandomEntropy = 11,
        AslrRegionAddress = 12,
        AslrRegionSize = 13,
        StackRegionAddress = 14,
        StackRegionSize = 15,
        SystemResourceSizeTotal = 16,
        SystemResourceSizeUsed = 17,
        ProgramId = 18,
        InitialProcessIdRange = 19,
        UserExceptionContextAddress = 20,
        TotalNonSystemMemorySize = 21,
        UsedNonSystemMemorySize = 22,
        IsApplication = 23,
    };

    // TODO: sub-IDs

    Result SetHeapSize(void *&out_addr, u64 size); // 0x01
    Result SetMemoryPermission(void *addr, u64 size, u32 perm); // 0x02
    Result SetMemoryAttribute(void *addr, u64 size, u32 mask, MemoryAttribute attr); // 0x03
    Result QueryMemory(MemoryInfo &out_info, u32 &out_page_info, u64 address); // 0x06
    void __attribute__((noreturn)) ExitProcess(); // 0x07
    Result CreateThread(Handle &out_handle, ThreadEntrypointFunction entry, void *entry_arg, void *stack_top, i32 priority, i32 cpu_id); // 0x08
    Result StartThread(Handle handle); // 0x09
    void __attribute__((noreturn)) ExitThread(); // 0x0A
    void SleepThread(i64 timeout_ns); // 0x0B
    Result GetThreadPriority(i32 &out_priority, Handle handle); // 0x0C
    Result SetThreadPriority(Handle handle, i32 priority); // 0x0D
    Result MapSharedMemory(u32 handle, void *address, u64 size, MemoryPermission perm); // 0x13
    Result UnmapSharedMemory(u32 handle, void *address, u64 size); // 0x14
    Result CreateTransferMemory(Handle &out_handle, void *address, u64 size, MemoryPermission permission); // 0x15
    Result CloseHandle(Handle handle); // 0x16
    Result WaitSynchronization(i32 &out_index, const u32 *handles, i32 handle_count, i64 timeout_ns); // 0x18
    Result ArbitrateLock(u32 wait_tag, u32 *tag_location, u32 self_tag); // 0x1A
    Result ArbitrateUnlock(u32 *tag_location); // 0x1B
    Result ConnectToNamedPort(Handle &out_handle, const char *name); // 0x1F
    Result SendSyncRequest(Handle handle); // 0x21
    Result GetProcessId(u64 &out_process_id, Handle handle); // 0x24
    Result GetThreadId(u64 &out_thread_id, Handle handle); // 0x25
    Result Break(u32 break_reason, u64 inval1, u64 inval2); // 0x26
    Result OutputDebugString(const char *str, u64 len); // 0x27
    void __attribute__((noreturn)) ReturnFromException(Result res);
    Result GetInfo(u64 &out_info, InfoId id, Handle handle, u64 sub_id); // 0x29
    Result CreateSession(u32 &out_server_handle, u32 &out_client_handle, bool is_light, u64 name); // 0x40
    Result AcceptSession(u32 &out_session_handle, u32 port_handle); // 0x41
    Result ReplyAndReceive(i32 &out_index, const u32 *handles, i32 handle_count, u32 reply_target, u64 timeout); // 0x43
    Result ManageNamedPort(u32 &out_handle, const char *name, i32 max_sessions); // 0x71

}

#include <bio/util/util_String.hpp>

#define DEBUG_LOG(msg) ::bio::svc::OutputDebugString(msg, BIO_UTIL_STRLEN(msg))
#define DEBUG_LOG_FMT(fmt, ...) ({ \
    char msg[0x400] = {}; \
    const auto len = static_cast<u32>(::bio::util::SPrintf(msg, fmt, ##__VA_ARGS__)); \
    ::bio::svc::OutputDebugString(msg, len); \
})
