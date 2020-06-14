
#pragma once
#include <bio/base.hpp>

#define BIO_RDEF_MODULE(val) constexpr u32 Module = val * 100

#define BIO_RDEF_DEFINE_RES(name, val) constexpr Result Result ## name(::bio::result::ResultModule, Module + val)

#define BIO_RDEF_LEGACY_MODULE(val) constexpr u32 LegacyModule = val

#define BIO_RDEF_LEGACY_DEFINE_RES(name, val) constexpr Result Result ## name(LegacyModule, val)

namespace bio::result {

    constexpr u32 ResultModule = 420;

    BIO_RDEF_MODULE(0);

    BIO_RDEF_DEFINE_RES(NroLibraryNotExecutable, 1);
    BIO_RDEF_DEFINE_RES(MemoryAllocationFailure, 2);

}