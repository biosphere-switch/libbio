
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    using AtExitFunction = void(*)(void*);
    using ExitFunction = void(*)(i32);

    constexpr i32 ExitSuccess = 0;

    void RegisterAtExit(AtExitFunction fn, void *arg = nullptr);
    void Exit(i32 error_code);

}

#define CRT0_RES_ASSERT(expr) ({ \
    const auto _tmp_rc = _BIO_AS_RESULT(expr); \
    if(_tmp_rc.IsFailure()) { \
        ::bio::crt0::Exit(_tmp_rc.GetValue()); \
    } \
})