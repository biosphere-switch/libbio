
#pragma once
#include <bio/input/input_SharedMemoryTypes.hpp>

namespace bio::input {

    enum class Key : u64 {
        A = BIO_BITMASK(0),
        B = BIO_BITMASK(1),
        X = BIO_BITMASK(2),
        Y = BIO_BITMASK(3),
        LStick = BIO_BITMASK(4),
        RStick = BIO_BITMASK(5),
        L = BIO_BITMASK(6),
        R = BIO_BITMASK(7),
        ZL = BIO_BITMASK(8),
        ZR = BIO_BITMASK(9),
        Plus = BIO_BITMASK(10),
        Minus = BIO_BITMASK(11),
        Left = BIO_BITMASK(12),
        Right = BIO_BITMASK(13),
        Up = BIO_BITMASK(14),
        Down = BIO_BITMASK(15),
        LStickLeft = BIO_BITMASK(16),
        LStickUp = BIO_BITMASK(17),
        LStickRight = BIO_BITMASK(18),
        LStickDown = BIO_BITMASK(19),
        RStickLeft = BIO_BITMASK(20),
        RStickUp = BIO_BITMASK(21),
        RStickRight = BIO_BITMASK(22),
        RStickDown = BIO_BITMASK(23),
        SLLeft = BIO_BITMASK(24),
        SRLeft = BIO_BITMASK(25),
        SLRight = BIO_BITMASK(26),
        SRRight = BIO_BITMASK(27),
        Touch = BIO_BITMASK(28),
    };

    BIO_ENUM_BIT_OPERATORS(Key, u64)

}