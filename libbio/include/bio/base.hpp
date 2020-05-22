
#pragma once

using u8 = unsigned char;
static_assert(sizeof(u8) == 1, "u8");

using u16 = unsigned short;
static_assert(sizeof(u16) == 2, "u16");

using u32 = unsigned int;
static_assert(sizeof(u32) == 4, "u32");

using u64 = unsigned long long;
static_assert(sizeof(u64) == 8, "u64");

using i8 = signed char;
static_assert(sizeof(i8) == 1, "i8");

using i16 = signed short;
static_assert(sizeof(i16) == 2, "i16");

using i32 = signed int;
static_assert(sizeof(i32) == 4, "i32");

using i64 = signed long long;
static_assert(sizeof(i64) == 8, "i64");

// TODO: future aarch32 support?

#ifdef __aarch64__
    #define BIO_TARGET_NAME aarch64
    #define BIO_TARGET_AARCH64
    using Size = u64;
#elif __arm__
    #define BIO_TARGET_NAME aarch32
    #define BIO_TARGET_AARCH32
    using Size = u32;
#else
    #error "Cannot determine the target architecture"
#endif

namespace bio::result::impl {

    static constexpr u32 ModuleBits = 9;
    static constexpr u32 DescriptionBits = 13;
    static constexpr u32 ReservedBits = 10;
    static constexpr u32 DefaultValue = u32();
    static constexpr u32 SuccessValue = DefaultValue;

    inline constexpr u32 Pack(u32 mod, u32 desc) {
        return mod | (desc << ModuleBits);
    }

    inline constexpr u32 UnpackModule(u32 value) {
        return value & ~(~DefaultValue << ModuleBits);
    }

    inline constexpr u32 UnpackDescription(u32 value) {
        return (value >> ModuleBits) & ~(~DefaultValue << DescriptionBits);
    }

}

namespace bio {

    struct Result {
        u32 value;

        constexpr Result() : value(result::impl::SuccessValue) {}
        constexpr Result(u32 value) : value(value) {}
        constexpr Result(u32 mod, u32 desc) : value(result::impl::Pack(mod, desc)) {}

        inline constexpr bool IsSuccess() const {
            return this->value == result::impl::SuccessValue;
        }

        inline constexpr bool IsFailure() const {
            return !this->IsSuccess();
        }

        inline constexpr u32 GetModule() const {
            return result::impl::UnpackModule(this->value);
        }

        inline constexpr u32 GetDescription() const {
            return result::impl::UnpackDescription(this->value);
        }

        inline constexpr u32 GetValue() const {
            return this->value;
        }
        
        inline constexpr operator u32() const {
            return this->GetValue();
        }

    };
    static_assert(sizeof(Result) == 4, "Result");

    constexpr Result ResultSuccess = result::impl::SuccessValue;

    constexpr u32 InvalidHandle = 0;

}

#define BIO_BITMASK(n) (1 << n)

#define BIO_IS_CONSTANT(v) (__builtin_constant_p(v) ? true : false)

#define _BIO_ENSURE_STR_LITERAL(str) "" str ""
#define BIO_ENSURE_STR_LITERAL(str) _BIO_ENSURE_STR_LITERAL(str)

#define _BIO_AS_RESULT static_cast<::bio::Result>

#define BIO_RES_TRY(expr) ({ \
    const auto _tmp_rc = (expr); \
    if(_BIO_AS_RESULT(_tmp_rc).IsFailure()) { \
        return _tmp_rc; \
    } \
})

#define BIO_RES_TRY_EXCEPT(expr, except, ...) ({ \
    auto _tmp_rc = _BIO_AS_RESULT(expr); \
    auto _tmp_expect = _BIO_AS_RESULT(expr); \
    if(_tmp_rc.GetValue() != except.GetValue()) { \
        if(_tmp_rc.IsFailure()) { \
            return _tmp_rc; \
        } \
    } \
})

#define BIO_RET_UNLESS(expr, ret) ({ \
    auto _tmp_ret = (ret); \
    if(!(expr)) { \
        return _tmp_ret; \
    } \
})

#define BIO_RET_UNLESS_EX(expr, ...) ({ \
    if(!(expr)) { \
        __VA_ARGS__ \
    } \
})

#define BIO_RET_IF(expr, ret) BIO_RET_UNLESS(!(expr), ret)
#define BIO_RET_IF_EX(expr, ...) BIO_RET_UNLESS_EX(!(expr), ##__VA_ARGS__)
