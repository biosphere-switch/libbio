
#pragma once
#include <bio/os/os_Thread.hpp>

namespace bio::os {

    constexpr u64 ThreadContextMagic = 0xFFFF534C544F4942; // BIOTLS(0xFF)(0xFF)

    struct ThreadContext {
        u64 bio_magic;
        Thread *thread_ref;

        constexpr ThreadContext() : thread_ref(nullptr), bio_magic(0) {}

        inline Result Initialize(u32 thread_handle, const char *name, void *stack, u64 stack_size, bool owns_stack) {
            BIO_RET_UNLESS(this->thread_ref != nullptr, 0xdead);
            BIO_RES_TRY(this->thread_ref->InitializeWith(thread_handle, name, stack, stack_size, owns_stack));

            this->bio_magic = ThreadContextMagic;
            return ResultSuccess;
        }

    };

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