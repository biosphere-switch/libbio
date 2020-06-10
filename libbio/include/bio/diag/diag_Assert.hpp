
#pragma once
#include <bio/diag/diag_Types.hpp>
#include <bio/diag/diag_Results.hpp>
#include <bio/util/util_String.hpp>

namespace bio::diag {

    enum class AssertMode : u8 {
        Default = BIO_BITMASK(0),
        SvcOutput = BIO_BITMASK(1),
        DiagLog = BIO_BITMASK(2),
        ProcessExit = BIO_BITMASK(3),
        Fatal = BIO_BITMASK(4),
        SvcBreak = BIO_BITMASK(5),
    };

    BIO_ENUM_BIT_OPERATORS(AssertMode, u8)

    struct AssertMetadata {
        SourceInfo source_info;
        const char *assertion_msg;
        u32 assertion_msg_len;
        Result assertion_rc;
        AssertMode assert_mode;
    };


    void AssertImpl(const AssertMetadata &metadata);

}

#define BIO_DIAG_DETAILED_ASSERT_BASE(mode, msg, msg_len, rc) ({ \
    const ::bio::diag::AssertMetadata _assert_metadata = { \
        .source_info = { \
            .line_number = __LINE__, \
            .file_name = __FILE__, \
            .file_name_len = __builtin_strlen(__FILE__), \
            .function_name = __func__, \
            .function_name_len = __builtin_strlen(__func__), \
        }, \
        .assertion_msg = msg, \
        .assertion_msg_len = msg_len, \
        .assertion_rc = rc, \
        .assert_mode = mode, \
    }; \
    ::bio::diag::AssertImpl(_assert_metadata); \
})

#define BIO_DIAG_DETAILED_RES_ASSERT(mode, rc) ({ \
    char msg[0x40] = {}; \
    const auto _rc = static_cast<::bio::Result>(rc); \
    if(_rc.IsFailure()) { \
        const auto len = static_cast<u32>(::bio::util::SPrintf(msg, "%s -> 0x%X (%04d-%04d)", #rc, _rc.GetValue(), _rc.GetModule() + 2000, _rc.GetDescription())); \
        BIO_DIAG_DETAILED_ASSERT_BASE(mode, msg, len, _rc); \
    } \
})

#define BIO_DIAG_DETAILED_ASSERT(mode, cond) ({ \
    const auto _cond = (cond); \
    if(!_cond) { \
        BIO_DIAG_DETAILED_ASSERT_BASE(mode, #cond, BIO_UTIL_STRLEN(#cond), ::bio::diag::result::ResultAssertionFailed); \
    } \
})

#define BIO_DIAG_RES_ASSERT(rc) BIO_DIAG_DETAILED_RES_ASSERT(::bio::diag::AssertMode::Default, rc)

#define BIO_DIAG_ASSERT(cond) BIO_DIAG_DETAILED_ASSERT(::bio::diag::AssertMode::Default, cond)
