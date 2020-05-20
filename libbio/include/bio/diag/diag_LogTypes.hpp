
#pragma once
#include <bio/base.hpp>

namespace bio::diag {

    enum class LogPacketFlags : u8 {
        Head = BIO_BITMASK(0),
        Tail = BIO_BITMASK(1),
        LittleEndian = BIO_BITMASK(2),
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
    static_assert(sizeof(LogPacketHeader) == 0x18, "LogPacketHeader");

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
    static_assert(sizeof(LogDataChunkHeader) == 2, "LogDataChunkHeader");

}