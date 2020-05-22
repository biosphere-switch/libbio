
#pragma once
#include <bio/base.hpp>

namespace bio::os {

    struct Mutex {
        u32 handle_value;
        bool recursive;
        u32 counter;
        u32 thread_handle;

        Mutex(bool recursive = false) : handle_value(InvalidHandle), recursive(recursive), counter(0), thread_handle(InvalidHandle) {}

        void Lock();

        void Unlock();

        bool TryLock();

        bool IsLockedByCurrentThread();

    };

    class ScopedMutexLock {

        private:
            Mutex &mtx;

        public:
            ScopedMutexLock(Mutex &mtx) : mtx(mtx) {
                this->mtx.Lock();
            }

            ~ScopedMutexLock() {
                this->mtx.Unlock();
            }

    };

}
