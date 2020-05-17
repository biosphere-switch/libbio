
#pragma once

namespace bio::util {

    template<typename T>
    struct RemoveRef {
        using Type = T;
    };

    template<typename T>
    struct RemoveRef<T&> {
        using Type = T;
    };

    template<typename T>
    struct RemoveRef<T&&> {
        using Type = T;
    };

    template<typename T>
    using NoRef = typename RemoveRef<T>::Type;

    template<typename T>
    NoRef<T> &&Move(T &&t) {
        return static_cast<NoRef<T>&&>(t);
    }

    template<typename T>
    inline void Swap(T &a, T &b) {
        T tmp = Move(a);
        a = Move(b);
        b = Move(tmp);
    }

}