
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/mem/mem_SharedObject.hpp>
#include <bio/os/os_Results.hpp>

namespace bio::os {

    enum class ThreadState : u32 {
        NotInitialized,
        Initialized,
        DestroyedBeforeStarted,
        Started,
        Terminated,
    };

    struct Thread {

        static constexpr i32 InvalidPriority = -1;
        static constexpr u32 NameMaxLength = 0x20;
        static constexpr i32 DefaultCpuId = -2;

        ThreadState state; // TODO: make use of this
        bool owns_stack;
        void *stack;
        u64 stack_size;
        void *entry_arg;
        svc::ThreadEntrypointFunction entry;
        void *tls_slots[0x20];
        char name[NameMaxLength + 1];
        u32 name_len;
        u32 handle;

        constexpr Thread() : state(ThreadState::NotInitialized), owns_stack(false), stack(nullptr), stack_size(0), entry_arg(nullptr), entry(nullptr), tls_slots(), name(), name_len(0), handle(InvalidHandle) {}
        constexpr Thread(svc::ThreadEntrypointFunction entry, void *entry_arg, void *stack, u64 stack_size, bool owns_stack) : state(ThreadState::NotInitialized), owns_stack(owns_stack), stack(stack), stack_size(stack_size), entry_arg(entry_arg), entry(entry), tls_slots(), name(), name_len(0), handle(InvalidHandle) {}

        inline Result EnsureStack() {
            if(this->stack == nullptr) {
                if(this->owns_stack) {
                    BIO_RES_TRY(mem::PageAllocate(this->stack_size, this->stack));
                }
            }
            return ResultSuccess;
        }

        Result EnsureCreated(i32 priority, i32 cpu_id);

        inline Result InitializeWith(u32 thread_handle, const char *name, void *stack, u64 stack_size, bool owns_stack) {
            this->SetName(name);

            this->stack = stack;
            this->stack_size = stack_size;
            this->owns_stack = owns_stack;
            this->handle = thread_handle;
            return ResultSuccess;
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
                util::Strncpy(this->name, name, NameMaxLength + 1);
                this->name_len = BIO_UTIL_STRLEN(name);
            }
        }

        inline char *GetName() {
            return this->name;
        }

        inline u32 GetNameLength() {
            return this->name_len;
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

}