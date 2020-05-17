
#pragma once

namespace bio::util {

    template<typename T>
    inline void Swap(T &a, T &b) {
        T tmp = a;
        a = b;
        b = tmp;
    }

}