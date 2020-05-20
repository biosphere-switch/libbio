
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    using ExitFunction = void(*)(i32 error_code);

    constexpr i32 SuccessExit = 0;

    void Exit(i32 error_code);
    void D();

}

#define CRT0_RES_ASSERT(expr) ({ \
    const auto _tmp_rc = _BIO_AS_RESULT(expr); \
    if(_tmp_rc.IsFailure()) { \
        ::bio::crt0::Exit(_tmp_rc.GetValue()); \
    } \
})