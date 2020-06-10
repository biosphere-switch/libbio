
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/diag/diag_Types.hpp>

namespace bio::diag {

    enum class LogSeverity : u8 {
        Trace,
        Info,
        Warn,
        Error,
        Fatal,
    };

    struct LogMetadata {
        SourceInfo source_info;
        LogSeverity severity;
        bool verbosity;
        const char *text_log;
        const u32 text_log_len;
    };

    Result LogImpl(const LogMetadata &metadata);

    #define BIO_DIAG_DETAILED_LOG(log_severity, log_verbosity, msg, msg_len) ({ \
        const ::bio::diag::LogMetadata _log_metadata = { \
            .source_info = { \
                .line_number = __LINE__, \
                .file_name = __FILE__, \
                .file_name_len = __builtin_strlen(__FILE__), \
                .function_name = __func__, \
                .function_name_len = __builtin_strlen(__func__), \
            }, \
            .severity = log_severity, \
            .verbosity = log_verbosity, \
            .text_log = msg, \
            .text_log_len = msg_len, \
        }; \
        ::bio::diag::LogImpl(_log_metadata); \
    })

}

#define BIO_DIAG_DETAILED_LOGF(log_severity, log_verbosity, fmt, ...) ({ \
    char msg[0x400] = {}; \
    const u32 msg_len = static_cast<u32>(::bio::util::SNPrintf(msg, sizeof(msg), fmt, ##__VA_ARGS__)); \
    BIO_DIAG_DETAILED_LOG(log_severity, log_verbosity, msg, msg_len); \
})

#define BIO_DIAG_DETAILED_VLOG(log_severity, log_verbosity, fmt, args) ({ \
    char msg[0x400] = {}; \
    const u32 msg_len = static_cast<u32>(::bio::util::VSNPrintf(msg, sizeof(msg), fmt, args)); \
    BIO_DIAG_DETAILED_LOG(log_severity, log_verbosity, msg, msg_len); \
})

#define BIO_DIAG_LOG(msg) BIO_DIAG_DETAILED_LOG(::bio::diag::LogSeverity::Info, false, BIO_ENSURE_STR_LITERAL(msg), __builtin_strlen(BIO_ENSURE_STR_LITERAL(msg)))

#define BIO_DIAG_LOGF(fmt, ...) BIO_DIAG_DETAILED_LOGF(::bio::diag::LogSeverity::Info, false, fmt, ##__VA_ARGS__)

#define BIO_DIAG_VLOG(fmt, args) BIO_DIAG_DETAILED_VLOG(::bio::diag::LogSeverity::Info, false, fmt, args)
