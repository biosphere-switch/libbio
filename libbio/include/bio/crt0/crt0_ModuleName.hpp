
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    struct ModuleName {
        u32 zero;
        u32 length;
        char name[0x200];
    };

    #define CRT0_MAKE_MODULE_NAME(mod_name) (::bio::crt0::ModuleName) { \
        .zero = 0, \
        .length = __builtin_strlen(mod_name), \
        .name = mod_name, \
    }

    #define CRT0_DEFINE_MODULE_NAME(mod_name) \
        __attribute__((used)) __attribute__((section(".module_name"))) \
        const auto g_ModuleName = CRT0_MAKE_MODULE_NAME(mod_name);

    /*
    Specifying a custom module name:

    __attribute__((section(".module_name")))
    volatile auto g_ModuleName = CRT0_MAKE_MODULE_NAME("custom-name");
    */
    
}