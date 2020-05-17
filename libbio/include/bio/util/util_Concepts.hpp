
#pragma once

namespace bio::util {

    template<typename T, typename U>
    struct Same {
        static constexpr bool Value = false;
    };

    template<typename T>
    struct Same<T, T> {
        static constexpr bool Value = true;
    };

    template<typename T, typename U>
    concept SameAs = Same<T, U>::Value;

}