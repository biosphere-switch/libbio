
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/mem/mem_SharedObject.hpp>
#include <bio/os/os_Results.hpp>

namespace bio::os {

    enum class ThreadState : u8 {
        NotInitialized,
        Initialized,
        DestroyedBeforeStarted,
        Started,
        Terminated,
    };

    using ThreadName = char[0x20];

    // Note: struct must match 0x1D0, name must be at offset 0x188 and name_addr at 0x1A8
    // TODO: the "reserved" fields are there to match size 0x1D0, use them for something...?

    struct Thread {

        static constexpr i32 InvalidPriority = -1;
        static constexpr u32 NameMaxLength = sizeof(ThreadName);
        static constexpr i32 DefaultCpuId = -2;

        Thread *this_addr;
        ThreadState state; // TODO: make use of this
        bool owns_stack;
        u8 pad[2];
        u32 handle;
        void *stack;
        u64 stack_size;
        void *entry_arg;
        svc::ThreadEntrypointFunction entry;
        void *tls_slots[0x20];
        u8 reserved[0x58];
        ThreadName name;
        ThreadName *name_addr;
        u8 reserved_2[0x20];

        constexpr Thread() : this_addr(this), state(ThreadState::NotInitialized), owns_stack(false), handle(InvalidHandle), stack(nullptr), stack_size(0), entry_arg(nullptr), entry(nullptr), tls_slots(), name(), name_addr(&name) {}
        constexpr Thread(svc::ThreadEntrypointFunction entry, void *entry_arg, void *stack, u64 stack_size, bool owns_stack) : this_addr(this), state(ThreadState::NotInitialized), owns_stack(owns_stack), handle(InvalidHandle), stack(stack), stack_size(stack_size), entry_arg(entry_arg), entry(entry), tls_slots(), name(), name_addr(&name) {}

        inline Result EnsureStack() {
            if(this->stack == nullptr) {
                if(this->owns_stack) {
                    BIO_RES_TRY(mem::Allocate<mem::PageAlignment>(this->stack_size, this->stack));
                }
            }
            return ResultSuccess;
        }

        Result EnsureCreated(i32 priority, i32 cpu_id);

        inline void InitializeWith(u32 thread_handle, const char *name, void *stack, u64 stack_size, bool owns_stack) {
            this->this_addr = this;
            this->SetName(name);
            this->stack = stack;
            this->stack_size = stack_size;
            this->owns_stack = owns_stack;
            this->handle = thread_handle;
            this->state = ThreadState::Started;
        }

        inline u32 GetHandle() {
            return this->handle;
        }

        inline Result GetId(u64 &out_id) {
            BIO_RES_TRY(svc::GetThreadId(out_id, this->handle));
            return ResultSuccess;
        }

        inline Result GetPriority(i32 &out_prio) {
            BIO_RES_TRY(svc::GetThreadPriority(out_prio, this->handle));
            return ResultSuccess;
        }

        inline Result SetPriority(i32 priority) {
            BIO_RES_TRY(svc::SetThreadPriority(this->handle, priority));
            return ResultSuccess;
        }

        inline void SetName(const char *name) {
            if(name != nullptr) {
                mem::ZeroArray(this->name);
                util::Strncpy(this->name, name, NameMaxLength);
                this->name_addr = &this->name;
            }
        }

        inline char *GetName() {
            return reinterpret_cast<char*>(this->name_addr);
        }

        inline constexpr u32 GetNameLength() {
            return BIO_UTIL_STRLEN(this->GetName());
        }

        inline Result Start() {
            BIO_RES_TRY(svc::StartThread(this->handle));
            return ResultSuccess;
        }

        inline void Dispose() {
            if(this->owns_stack) {
                if(this->stack != nullptr) {
                    mem::Free(this->stack);
                    this->stack = nullptr;
                }
            }
            svc::CloseHandle(this->handle);
            this->handle = InvalidHandle;
        }

        static Result Create(svc::ThreadEntrypointFunction entry, void *entry_arg, void *stack, u64 stack_size, i32 priority, i32 cpu_id, const char *name, mem::SharedObject<Thread> &out_thread);

    };
    static_assert(__builtin_offsetof(Thread, name) == 0x188);
    static_assert(__builtin_offsetof(Thread, name_addr) == 0x1A8);
    static_assert(sizeof(Thread) == 0x1D0);

}