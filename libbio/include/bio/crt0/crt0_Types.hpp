
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

}