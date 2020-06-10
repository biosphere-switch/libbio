
#pragma once
#include <bio/base.hpp>

namespace bio::service::nv {

    enum class ErrorCode : u32 {
        Success = 0,
        NotImplemented = 1,
        NotSupported = 2,
        NotInitialized = 3,
        InvalidParameter = 4,
        TimeOut = 5,
        InsufficientMemory = 6,
        ReadOnlyAttribute = 7,
        InvalidState = 8,
        InvalidAddress = 9,
        InvalidSize = 10,
        InvalidValue = 11,
        AlreadyAllocated = 13,
        Busy = 14,
        ResourceError = 15,
        CountMismatch = 16,
        SharedMemoryTooSmall = 0x1000,
        FileOperationFailed = 0x30003,
        IoctlFailed = 0x3000F,
    };

    // TODO: support more ioctls

    enum class IoctlId : u32 {
        NvMapCreate = 0xC0080101,
        NvMapFromId = 0xC0080103,
        NvMapAlloc = 0xC0200104,
        NvMapFree = 0xC0180105,
        NvMapParam = 0xC00C0109,
        NvMapGetId = 0xC008010E,

        NvHostCtrlWaitAsync = 0xC00C0016,
    };

}