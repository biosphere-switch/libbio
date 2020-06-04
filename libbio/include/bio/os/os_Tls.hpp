
#pragma once
#include <bio/os/os_Thread.hpp>

namespace bio::os {

    struct ThreadLocalStorage {
        u8 ipc_buffer[0x100];
        u32 preemption_state;
        u8 unk[0xF4];
        Thread *thread_addr;
    };
    static_assert(__builtin_offsetof(ThreadLocalStorage, thread_addr) == 0x1F8);
    static_assert(sizeof(ThreadLocalStorage) == 0x200);

    ThreadLocalStorage *GetThreadLocalStorage();

    inline Thread &GetCurrentThread() {
        return *GetThreadLocalStorage()->thread_addr;
    }

}