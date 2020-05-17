
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    struct ModuleName {
        u32 zero;
        u32 length;
        char module[0x200];
    };

    #define CRT0_MAKE_MODULE_NAME(name) (::bio::crt0::ModuleName) { \
        .zero = 0, \
        .length = __builtin_strlen(name), \
        .module = name, \
    }

    #define CRT0_DEFINE_MODULE_NAME(name) \
        __attribute__((used)) __attribute__((section(".module_name"))) \
        const auto g_ModuleName = CRT0_MAKE_MODULE_NAME(name);

    /*
    Weak symbol to override in order to specify the module name:
    ModuleName g_ModuleName = _CRT0_MAKE_MODULE_NAME("nostd");
    */
    
}