#include <bio/crt0/crt0_Exit.hpp>

namespace bio::crt0 {

    ExitFunction g_ExitFunction = nullptr;

    void Exit(i32 error_code) {
        // g_ExitFunction must have a valid value, which is set by the CRT0
        g_ExitFunction(error_code);
    }

}