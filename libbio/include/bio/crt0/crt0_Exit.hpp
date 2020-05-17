
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    using ExitFunction = void(*)(i32 error_code);

    constexpr i32 SuccessExit = 0;

    void Exit(i32 error_code);

}