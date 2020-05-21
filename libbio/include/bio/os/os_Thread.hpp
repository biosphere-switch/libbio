
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/svc/svc_Impl.hpp>

namespace bio::os {

    typedef void(*ThreadEntrypoint)(void*);

    constexpr u32 MaxThreadNameLength = 0x20;

    enum class ThreadState : u32 {
        NotInitialized,
        Initialized,
        DestroyedBeforeStarted,
        Started,
        Terminated,
    };

    struct ThreadInfo {
        u8 unk_reserved[0x34]; // This is made so that thread name is at offset 0x188
        u32 state;
        i32 priority;
        bool owns_stack;
        u8 pad[3];
        u64 id;
        void *stack;
        u64 stack_size;
        void *entry_arg;
        ThreadEntrypoint entry;
        void *tls_slots[0x20];
        char name[MaxThreadNameLength];
        void *name_addr;
        u32 unk_critical_section;
        u32 unk_cond_var;
        u32 handle;
        u32 name_length;
        u64 unk_mutex;

        constexpr ThreadInfo() : state(static_cast<u32>(ThreadState::NotInitialized)), priority(0), owns_stack(false), stack(nullptr), stack_size(0) {}

        inline Result InitializeWith(u32 thread_handle, const char *name) {
            // Get thread ID.
            BIO_RES_TRY(svc::GetThreadId(this->id, thread_handle));
            // Get priority.
            BIO_RES_TRY(svc::GetThreadPriority(this->priority, thread_handle));

            // Get the stack memory region.
            svc::MemoryInfo info;
            u32 page_info;
            BIO_RES_TRY(svc::QueryMemory(info, page_info, reinterpret_cast<u64>(&info)));

            this->SetThreadName(name);

            this->stack = reinterpret_cast<void*>(info.address);
            this->stack_size = info.size;
            this->owns_stack = false;
            this->handle = thread_handle;
            return ResultSuccess;
        }

        inline u64 GetThreadId() {
            return this->id;
        }

        inline i32 GetThreadPriority() {
            return this->priority;
        }

        inline void SetThreadName(const char *name) {
            this->name_length = util::SNPrintf(this->name, MaxThreadNameLength, "%s", name);
            this->name_addr = reinterpret_cast<u8*>(this) + __builtin_offsetof(ThreadInfo, name);
        }

        inline char *GetThreadName() {
            return reinterpret_cast<char*>(this->name_addr);
        }

        inline u32 GetThreadNameLength() {
            return this->name_length;
        }

    };
    static_assert(sizeof(ThreadInfo) == 0x1A8);

    constexpr u64 ThreadContextMagic = 0xFFFF534C544F4942; // BIOTLS(0xFF)(0xFF)

    struct ThreadContext {
        void *unk1;
        void *unk2;
        void *unk3;
        void *unk4;
        ThreadInfo thread_info;
        u64 bio_magic;

        constexpr ThreadContext() : unk1(nullptr), unk2(nullptr), unk3(nullptr), unk4(nullptr), thread_info(), bio_magic(0) {}

        inline Result Initialize(u32 thread_handle, const char *name) {
            BIO_RES_TRY(this->thread_info.InitializeWith(thread_handle, name));

            this->bio_magic = ThreadContextMagic;
            this->unk1 = this;
            return ResultSuccess;
        }

    };
    static_assert(sizeof(ThreadContext) == 0x1D0);

    struct ThreadLocalStorage {
        u32 ipc_buffer[0x40];
        u32 preemption_state;
        u8 unk[0xF4];
        ThreadContext *context;
    };
    static_assert(__builtin_offsetof(ThreadLocalStorage, context) == 0x1F8);
    static_assert(sizeof(ThreadLocalStorage) == 0x200);

    void *GetThreadLocalStorageValue();

    template<typename T = void>
    inline T *GetThreadLocalStorage() {
        return reinterpret_cast<T*>(GetThreadLocalStorageValue());
    }

    inline ThreadContext *GetThreadContext() {
        return GetThreadLocalStorage<ThreadLocalStorage>()->context;
    }

    inline ThreadInfo &GetCurrentThreadInfo() {
        return GetThreadContext()->thread_info;
    }

}