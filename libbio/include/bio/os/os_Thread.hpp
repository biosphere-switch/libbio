
#pragma once
#include <bio/util/util_String.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/mem/mem_SharedObject.hpp>
#include <bio/os/os_Results.hpp>

namespace bio::os {

    typedef void(*ThreadEntrypoint)(void*);

    constexpr i32 InvalidPriority = 0xFFFFFFFF;

    enum class ThreadState : u32 {
        NotInitialized,
        Initialized,
        DestroyedBeforeStarted,
        Started,
        Terminated,
    };

    struct Thread {

        static constexpr u32 NameMaxLength = 0x20;

        u32 state; // TODO: make use of this
        i32 priority;
        bool owns_stack;
        u64 id;
        void *stack;
        u64 stack_size;
        void *entry_arg;
        ThreadEntrypoint entry;
        void *tls_slots[0x20];
        char name[NameMaxLength + 1];
        u32 name_len;
        u32 handle;

        constexpr Thread() : state(static_cast<u32>(ThreadState::NotInitialized)), priority(0), owns_stack(false), stack(nullptr), stack_size(0) {}
        constexpr Thread(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, bool owns_stack, i32 priority) : state(static_cast<u32>(ThreadState::NotInitialized)), priority(priority), owns_stack(owns_stack), stack(stack), stack_size(stack_size), entry_arg(entry_arg), entry(entry) {}

        inline Result InitializeWith(u32 thread_handle, const char *name, void *stack, u64 stack_size, bool owns_stack) {
            // Get thread ID.
            BIO_RES_TRY(svc::GetThreadId(this->id, thread_handle));

            // Get priority.
            BIO_RES_TRY(svc::GetThreadPriority(this->priority, thread_handle));

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

        inline u64 GetId() {
            return this->id;
        }

        inline i32 GetPriority() {
            return this->priority;
        }

        inline void SetName(const char *name) {
            mem::ZeroArray(this->name);
            util::Strncpy(this->name, name, NameMaxLength + 1);
            this->name_len = BIO_UTIL_STRLEN(name);
        }

        inline char *GetName() {
            return this->name;
        }

        inline u32 GetNameLength() {
            return this->name_len;
        }

        // Functionality

        inline Result Start() {
            return svc::StartThread(this->handle);
        }

        inline void Dispose() {
            if(this->owns_stack) {
                if(this->stack != nullptr) {
                    mem::Free(this->stack);
                    this->stack = nullptr;
                }
            }
            svc::CloseHandle(this->handle);
        }

    };

    class ThreadObject {

        public:
            static constexpr i32 DefaultCpuId = -2;

        private:
            Thread thread;

        public:
            constexpr ThreadObject(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, bool owns_stack, i32 priority) : thread(entry, entry_arg, stack, stack_size, owns_stack, priority) {}

            ~ThreadObject() {
                this->thread.Dispose();
            }

            inline Thread &GetThread() {
                return this->thread;
            }

            inline Result Start() {
                return this->thread.Start();
            }

            static Result Create(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, i32 priority, i32 cpu_id, const char *name, mem::SharedObject<ThreadObject> &out_thread);

            inline static Result Create(ThreadEntrypoint entry, void *entry_arg, u64 stack_size, i32 priority, const char *name, mem::SharedObject<ThreadObject> &out_thread) {
                return Create(entry, entry_arg, nullptr, stack_size, priority, DefaultCpuId, name, out_thread);
            }

            inline static Result Create(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, i32 priority, const char *name, mem::SharedObject<ThreadObject> &out_thread) {
                return Create(entry, entry_arg, stack, stack_size, priority, DefaultCpuId, name, out_thread);
            }

            inline static Result Create(ThreadEntrypoint entry, void *entry_arg, u64 stack_size, const char *name, mem::SharedObject<ThreadObject> &out_thread) {
                return Create(entry, entry_arg, nullptr, stack_size, InvalidPriority, DefaultCpuId, name, out_thread);
            }

            inline static Result Create(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, const char *name, mem::SharedObject<ThreadObject> &out_thread) {
                return Create(entry, entry_arg, stack, stack_size, InvalidPriority, DefaultCpuId, name, out_thread);
            }

    };

}