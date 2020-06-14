
#pragma once
#include <bio/base.hpp>

namespace bio::crt0 {

    enum class ExceptionDescription : u32 {
        InstructionAbort = 0x100,
        Other = 0x101,
        MisalignedPc = 0x102,
        MisalignedSp = 0x103,
        Trap = 0x104,
        SError = 0x106,
        BadSvc = 0x301,
    };

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