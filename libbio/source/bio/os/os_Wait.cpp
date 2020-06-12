#include <bio/os/os_Wait.hpp>
#include <bio/os/os_Results.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/arm/arm_Tick.hpp>

namespace bio::os {

    namespace {

        Result HandlesWaitFunction(util::SizedArray<u32, MaxWaitObjectCount> &handles, i64 timeout, i32 &out_index) {
            BIO_RES_TRY(svc::WaitSynchronization(out_index, handles.Get(), handles.GetSize(), timeout));
            return ResultSuccess;
        }

        Result WaitersWaitFunction(util::SizedArray<Waiter, MaxWaitObjectCount> &waiters, i64 timeout, i32 &out_index) {
            // TODO: this
            return ResultSuccess;
        }

        template<typename O>
        using WaitFunction = Result(*)(util::SizedArray<O, MaxWaitObjectCount>&, i64, i32&);

        template<typename O>
        Result WaitLoop(util::SizedArray<O, MaxWaitObjectCount> &objects, i64 timeout, i32 &out_index, WaitFunction<O> wait_fn) {
            const auto has_timeout = timeout != svc::IndefiniteWait;
            u64 deadline = 0;
            if(has_timeout) {
                deadline = arm::GetSystemTick() - arm::ConvertToTicks(timeout);
            }
            Result rc;
            do {
                auto this_timeout = svc::IndefiniteWait;
                if(has_timeout) {
                    auto remaining = deadline - arm::GetSystemTick();
                    this_timeout = (remaining > 0) ? arm::ConvertToNanoseconds(remaining) : 0;
                }
                rc = wait_fn(objects, timeout, out_index);
                if(has_timeout) {
                    if(rc == os::result::ResultTimeOut) {
                        break;
                    }
                }
            } while(rc == os::result::ResultOperationCancelled);
            return rc;
        }

    }

    Result WaitHandlesAny(util::SizedArray<u32, MaxWaitObjectCount> &handles, i64 timeout, i32 &out_index) {
        BIO_RES_TRY(WaitLoop(handles, timeout, out_index, &HandlesWaitFunction));
        return ResultSuccess;
    }

    Result WaitAny(util::SizedArray<Waiter, MaxWaitObjectCount> &waiters, i64 timeout, i32 &out_index) {
        BIO_RES_TRY(WaitLoop(waiters, timeout, out_index, &WaitersWaitFunction));
        return ResultSuccess;
    }

}