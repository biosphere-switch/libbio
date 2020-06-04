#include <bio/os/os_Thread.hpp>
#include <bio/os/os_Tls.hpp>

namespace bio::os {

    namespace {

        void ThreadEntry(void *thread_v) {
            auto thread = reinterpret_cast<Thread*>(thread_v);

            // Set thread in thread context.
            auto tls = os::GetThreadLocalStorage();
            tls->thread_addr = thread;

            // Call actual entry.
            thread->entry(thread->entry_arg);

            // Finalize execution.
            svc::ExitThread();
        };

    }
    
    Result Thread::EnsureCreated(i32 priority, i32 cpu_id) {
        BIO_RES_TRY(this->EnsureStack());
        if(this->handle == InvalidHandle) {
            auto prio = priority;
            if(prio == InvalidPriority) {
                // Use current thread's priority.
                BIO_RES_TRY(os::GetCurrentThread().GetPriority(prio));
            }
            BIO_RES_TRY(svc::CreateThread(this->handle, &ThreadEntry, this, reinterpret_cast<u8*>(this->stack) + this->stack_size, prio, cpu_id));
        }
        return ResultSuccess;
    }

    Result Thread::Create(svc::ThreadEntrypointFunction entry, void *entry_arg, void *stack, u64 stack_size, i32 priority, i32 cpu_id, const char *name, mem::SharedObject<Thread> &out_thread) {
        const bool owns_stack = stack == nullptr;
        
        mem::SharedObject<Thread> thread;
        BIO_RES_TRY(mem::NewShared<Thread>(thread, entry, entry_arg, stack, stack_size, owns_stack));
        BIO_RES_TRY(thread->EnsureCreated(priority, cpu_id));

        thread->SetName(name);
        out_thread = util::Move(thread);
        return ResultSuccess;
    }

}