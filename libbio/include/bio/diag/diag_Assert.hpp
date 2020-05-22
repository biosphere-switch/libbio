
#pragma once
#include <bio/diag/diag_Types.hpp>
#include <bio/diag/diag_Results.hpp>
#include <bio/util/util_String.hpp>

namespace bio::diag {

    enum class AssertMode {
        Default,
        DiagLog,
        ProcessExit,
        Fatal,
        SvcBreak,
    };

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
    char msg[0x40]; \
    ::bio::mem::ZeroArray(msg); \
    const auto _rc = static_cast<::bio::Result>(rc); \
    const auto len = static_cast<u32>(::bio::util::SPrintf(msg, "Result assertion failed: 0x%X (%04d-%04d)", _rc.GetValue(), _rc.GetModule() + 2000, _rc.GetDescription())); \
    BIO_DIAG_DETAILED_ASSERT_BASE(mode, msg, len, _rc); \
})

#define BIO_DIAG_DETAILED_ASSERT(mode, cond) ({ \
    const auto _cond = (cond); \
    if(!_cond) { \
        BIO_DIAG_DETAILED_ASSERT_BASE(mode, #cond, BIO_UTIL_STRLEN(#cond), ::bio::diag::result::ResultAssertionFailed); \
    } \
})

#define BIO_DIAG_RES_ASSERT(rc) BIO_DIAG_DETAILED_RES_ASSERT(::bio::diag::AssertMode::Default, rc)

#define BIO_DIAG_ASSERT(cond) BIO_DIAG_DETAILED_ASSERT(::bio::diag::AssertMode::Default, cond)
