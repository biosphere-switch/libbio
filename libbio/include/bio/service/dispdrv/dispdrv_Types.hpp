
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::dispdrv {

    enum class RefcountType : i32 {
        Weak,
        Strong,
    };

    enum class ParcelTransactionId : u32 {
        RequestBuffer = 1,
        SetBufferCount = 2,
        DequeueBuffer = 3,
        DetachBuffer = 4,
        DetachNextBuffer = 5,
        AttachBuffer = 6,
        QueueBuffer = 7,
        CancelBuffer = 8,
        Query = 9,
        Connect = 10,
        Disconnect = 11,
        SetSidebandStream = 12,
        AllocateBuffers = 13,
        SetPreallocatedBuffer = 14,
    };

}