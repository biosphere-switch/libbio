
#pragma once
#include <bio/svc/svc_Impl.hpp>
#include <bio/util/util_Array.hpp>

namespace bio::os {

    enum class WaiterType {
        Handle,
        HandleWithClear,
    };

    constexpr u32 MaxWaitObjectCount = 0x40;

    struct Waiter {
        u32 handle;
        WaiterType type;

        inline constexpr bool IsValid() {
            return this->handle != InvalidHandle;
        }

        inline constexpr Waiter Create(u32 handle) {
            return { handle, WaiterType::Handle };
        }

        inline constexpr Waiter CreateWithClear(u32 handle) {
            return { handle, WaiterType::HandleWithClear };
        }

    };

    Result WaitHandlesAny(util::SizedArray<u32, MaxWaitObjectCount> &handles, i64 timeout, i32 &out_index);
    Result WaitAny(util::SizedArray<Waiter, MaxWaitObjectCount> &waiters, i64 timeout, i32 &out_index);

    template<typename ...Waitables>
    Result Wait(i64 timeout, i32 &out_index, Waitables &&...waitables) {
        static_assert(sizeof...(Waitables) <= MaxWaitObjectCount);

        util::SizedArray<Waiter, MaxWaitObjectCount> waiters;
        (waiters.Push(waitables), ...);
        BIO_RES_TRY(WaitAny(waiters, timeout, out_index));

        return ResultSuccess;
    }

    // TODO: WaitHandles (like above, with a template parameter pack)

}