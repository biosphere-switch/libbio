#include <bio/os/os_Thread.hpp>
#include <bio/os/os_Tls.hpp>

namespace bio::os {

    namespace {

        void ThreadEntry(void *arg) {
            DEBUG_LOG_FMT("Thread started! ptr: %p", arg);
            auto thread = reinterpret_cast<Thread*>(arg);

            // Set thread in thread context.
            auto tls = os::GetThreadLocalStorage<os::ThreadLocalStorage>();
            tls->thread_ref = thread;

            // Call actual entry.
            thread->entry(thread->entry_arg);

            // Finalize execution.
            svc::ExitThread();
        };

    }

    Result ThreadObject::Create(ThreadEntrypoint entry, void *entry_arg, void *stack, u64 stack_size, i32 priority, i32 cpu_id, const char *name, mem::SharedObject<ThreadObject> &out_thread) {
        const bool owns_stack = stack == nullptr;
        auto stack_ptr = stack;
        if(owns_stack) {
            stack_ptr = mem::Allocate(stack_size);
        }

        auto prio = priority;
        if(priority == InvalidPriority) {
            // Use current thread's priority.
            prio = os::GetCurrentThread().GetPriority();
        }

        DEBUG_LOG_FMT("CreateThread -> prio: %d", prio);

        auto thread_obj = mem::NewShared<ThreadObject>(entry, entry_arg, stack_ptr, stack_size, owns_stack, priority);
        auto &thread = thread_obj->GetThread();
        
        u32 handle = 0;
        BIO_RES_TRY(svc::CreateThread(handle, reinterpret_cast<void*>(&ThreadEntry), &thread, reinterpret_cast<u8*>(stack_ptr) + stack_size, prio, cpu_id));

        BIO_RES_TRY(thread.InitializeWith(handle, name, stack_ptr, stack_size, owns_stack));
        out_thread = util::Move(thread_obj);
        return ResultSuccess;
    }

}