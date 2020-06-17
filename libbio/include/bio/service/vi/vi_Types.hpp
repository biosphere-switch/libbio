
#pragma once
#include <bio/base.hpp>

namespace bio::service::vi {

    struct DisplayName {
        char name[0x40];
    };

    enum class LayerFlags : u32 {
        None = 0,
        Default = BIO_BITMASK(0),
    };

    BIO_ENUM_BIT_OPERATORS(LayerFlags, u32)

}