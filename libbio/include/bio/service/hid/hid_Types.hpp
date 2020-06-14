
#pragma once
#include <bio/base.hpp>

namespace bio::service::hid {

    enum class NpadStyleTag : u32 {
        ProController = BIO_BITMASK(0),
        Handheld = BIO_BITMASK(1),
        JoyconPair = BIO_BITMASK(2),
        JoyconLeft = BIO_BITMASK(3),
        JoyconRight = BIO_BITMASK(4),
        SystemExt = BIO_BITMASK(29),
        System = BIO_BITMASK(30),
    };

    BIO_ENUM_BIT_OPERATORS(NpadStyleTag, u32)

    enum class NpadJoyDeviceType : i64 {
        Left,
        Right,
    };

    enum class ControllerId : u32 {
        Player1 = 0,
        Player2 = 1,
        Player3 = 2,
        Player4 = 3,
        Player5 = 4,
        Player6 = 5,
        Player7 = 6,
        Player8 = 7,
        Handheld = 0x20,
    };

}