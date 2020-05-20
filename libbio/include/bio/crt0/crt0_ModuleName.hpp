
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    struct ModuleName {
        u32 zero;
        u32 length;
        char name[0x200];
    };

}

#define BIO_CRT0_MAKE_MODULE_NAME(mod_name) (::bio::crt0::ModuleName) { \
    .zero = 0, \
    .length = __builtin_strlen(BIO_ENSURE_STR_LITERAL(mod_name)), \
    .name = BIO_ENSURE_STR_LITERAL(mod_name), \
}

// Overrides the default module name symbol ("libbio") with a custom name
// Note: should be used in the global scope.
#define BIO_CRT0_DEFINE_MODULE_NAME(mod_name) namespace bio::crt0 { \
    __attribute__((used)) __attribute__((section(".module_name"))) \
    volatile auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME(BIO_ENSURE_STR_LITERAL(mod_name)); \
}