
#pragma once
#include <bio/base.hpp>

#define RES_DEF_MODULE(val) static constexpr u32 Module = val

#define RES_DEF_DEFINE(name, val) static constexpr Result Result ## name(::bio::result::ResultModule, Module + val)

namespace bio::result {

    static const u32 ResultModule = 420;

    RES_DEF_MODULE(0);

    RES_DEF_DEFINE(NroLibraryNotExecutable, 1);

}