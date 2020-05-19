
#pragma once
#include <bio/base.hpp>

namespace bio::diag {

    enum class LogPacketFlags : u8 {
        Head = (1 << 0),
        Tail = (1 << 1),
        LittleEndian = (1 << 2),
    };

    struct LogPacketHeader {
        u64 process_id;
        u64 thread_id;
        u8 flags;
        u8 pad;
        u8 severity;
        u8 verbosity;
        u32 payload_size;
    };
    static_assert(sizeof(LogPacketHeader) == 0x18, "a");

    enum class LogDataChunkKey : u8 {
        LogSessionBegin,
        LogSessionEnd,
        TextLog,
        LineNumber,
        FileName,
        FunctionName,
        ModuleName,
        ThreadName,
        LogPacketDropCount,
        UserSystemClock,
        ProcessName,
    };

    struct LogDataChunkHeader {
        u8 key;
        u8 length;
    };
    static_assert(sizeof(LogDataChunkHeader) == 2, "a");

}