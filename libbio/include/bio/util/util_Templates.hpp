
#pragma once

namespace bio::util {

    template<typename T>
    struct NoRefBase {
        using Type = T;
    };

    template<typename T>
    struct NoRefBase<T&> {
        using Type = T;
    };

    template<typename T>
    struct NoRefBase<T&&> {
        using Type = T;
    };

    template<typename T>
    using NoRef = typename NoRefBase<T>::Type;

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

    template<typename T>
    inline T &Min(T &a, T &b) {
        if(a < b) {
            return a;
        }
        return b;
    }

    template<typename T>
    inline T &Max(T &a, T &b) {
        if(a > b) {
            return a;
        }
        return b;
    }

    template<typename T>
    struct IsPointerBase {
        static constexpr bool Value = false;
    };

    template<typename T>
    struct IsPointerBase<T*> {
        static constexpr bool Value = true;
    };

    template<typename T>
    using IsPointer = typename IsPointerBase<T>::Value;

}