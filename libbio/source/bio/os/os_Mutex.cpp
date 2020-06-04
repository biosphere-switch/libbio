#include <bio/os/os_Mutex.hpp>
#include <bio/os/os_Tls.hpp>

namespace bio::os {

    // Note: grabbed from libnx's mutex implementation (and similar to N's).

    namespace {

        constexpr u32 HandleWaitMask = 0x40000000;

        inline u32 GetCurrentThreadHandle() {
            return GetCurrentThread().GetHandle();
        }

        inline u32 LoadExclusive(u32 *ptr) {
            u32 value;
            __asm__ __volatile__("ldaxr %w[value], %[ptr]" : [value]"=&r"(value) : [ptr]"Q"(*ptr) : "memory");
            return value;
        }

        inline i32 StoreExclusive(u32 *ptr, u32 value) {
            i32 result;
            __asm__ __volatile__("stlxr %w[result], %w[value], %[ptr]" : [result]"=&r"(result) : [value]"r"(value), [ptr]"Q"(*ptr) : "memory");
            return result;
        }

        inline void ClearExclusive() {
            __asm__ __volatile__("clrex" ::: "memory");
        }

        void LockBase(u32 *handle_value) {
            const auto thr_handle = GetCurrentThreadHandle();
    
            auto value = LoadExclusive(handle_value);
            while(true) {
                if(value == InvalidHandle) {
                    if(StoreExclusive(handle_value, thr_handle) != 0) {
                        value = LoadExclusive(handle_value);
                        continue;
                    }
                    break;
                }

                if((value & HandleWaitMask) == 0) {
                    if(StoreExclusive(handle_value, value | HandleWaitMask) != 0) {
                        value = LoadExclusive(handle_value);
                        continue;
                    }
                }

                svc::ArbitrateLock(value & ~HandleWaitMask, handle_value, thr_handle);

                value = LoadExclusive(handle_value);
                if((value & ~HandleWaitMask) == thr_handle) {
                    ClearExclusive();
                    break;
                }
            }
        }

        void UnlockBase(u32 *handle_value) {
            const auto thr_handle = GetCurrentThreadHandle();

            auto value = LoadExclusive(handle_value);
            while(true) {
                if(value != thr_handle) {
                    ClearExclusive();
                    break;
                }

                if(StoreExclusive(handle_value, InvalidHandle) == 0) {
                    break;
                }

                value = LoadExclusive(handle_value);
            }

            if(value & HandleWaitMask) {
                svc::ArbitrateUnlock(handle_value);
            }
        }

        bool TryLockBase(u32 *handle_value) {
            const auto thr_handle = GetCurrentThreadHandle();

            while(true) {
                auto value = LoadExclusive(handle_value);
                if(value != InvalidHandle) {
                    break;
                }

                if(StoreExclusive(handle_value, thr_handle) == 0) {
                    return true;
                }
            }

            ClearExclusive();
            return false;
        }

    }

    void Mutex::Lock() {
        bool do_lock = true;
        if(this->recursive) {
            do_lock = false;
            const auto thr_handle = GetCurrentThreadHandle();
            if(this->thread_handle != thr_handle) {
                do_lock = true;
                this->thread_handle = thr_handle;
            }
            this->counter++;
        }

        if(do_lock) {
            LockBase(&this->handle_value);
        }
    }

    void Mutex::Unlock() {
        bool do_unlock = true;
        if(this->recursive) {
            do_unlock = false;
            if((--this->counter) == 0) {
                this->thread_handle = InvalidHandle;
                do_unlock = true;
            }
        }

        if(do_unlock) {
            UnlockBase(&this->handle_value);
        }
    }

    bool Mutex::TryLock() {
        bool do_trylock = true;
        if(this->recursive) {
            do_trylock = false;
            const auto thr_handle = GetCurrentThreadHandle();
            if(this->thread_handle != thr_handle) {
                if(!TryLockBase(&this->handle_value)) {
                    return false;
                }
                this->thread_handle = thr_handle;
            }
            this->counter++;
            return true;
        }
        else {
            return TryLockBase(&this->handle_value);
        }
    }

    bool Mutex::IsLockedByCurrentThread() {
        const auto thr_handle = GetCurrentThreadHandle();
        return (this->handle_value & ~HandleWaitMask) == thr_handle;
    }

}