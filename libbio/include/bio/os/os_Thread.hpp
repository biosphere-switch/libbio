
#pragma once
#include <bio/base.hpp>

namespace bio::os {

    typedef void(*ThreadEntrypoint)(void*);

    struct ThreadBlock {
        // TODO
    };

    struct ThreadLocalStorage {
        u32 ipc_buffer[0x40];
        u32 preemption_state;
        u8 unk[0xF4];
        ThreadBlock *thread;
    };

    void *GetThreadLocalStorageValue();

    template<typename T>
    inline T *GetThreadLocalStorage() {
        return reinterpret_cast<T*>(GetThreadLocalStorageValue());
    }

}