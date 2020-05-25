
#pragma once
#include <bio/os/os_Thread.hpp>

namespace bio::os {

    struct ThreadLocalStorage {
        u32 ipc_buffer[0x40];
        u32 preemption_state;
        u8 unk[0xF4];
        Thread *thread_ref;
    };
    static_assert(__builtin_offsetof(ThreadLocalStorage, thread_ref) == 0x1F8);
    static_assert(sizeof(ThreadLocalStorage) == 0x200);

    void *GetThreadLocalStorageValue();

    template<typename T = void>
    inline T *GetThreadLocalStorage() {
        return reinterpret_cast<T*>(GetThreadLocalStorageValue());
    }

    inline Thread &GetCurrentThread() {
        return *GetThreadLocalStorage<os::ThreadLocalStorage>()->thread_ref;
    }

}