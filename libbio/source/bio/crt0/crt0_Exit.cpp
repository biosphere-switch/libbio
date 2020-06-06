#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/util/util_List.hpp>
#include <bio/os/os_Mutex.hpp>

namespace bio::crt0 {

    namespace {

        struct AtExitEntry {
            AtExitFunction fn;
            void *args;

            inline void Run() {
                if(this->fn != nullptr) {
                    (this->fn)(this->args);
                }
            }

        };

        util::LinkedList<AtExitEntry> g_AtExitEntries;
        os::Mutex g_AtExitLock;

        void ProcessAtExitEntries() {
            os::ScopedMutexLock lk(g_AtExitLock);
            for(u32 i = 0; i < g_AtExitEntries.GetSize(); i++) {
                auto &entry = g_AtExitEntries.GetAt(i);
                entry.Run();
            }
            g_AtExitEntries.Clear();
        }

        // Use svc::ExitProcess by default
        auto g_ExitFunction = reinterpret_cast<ExitFunction>(&svc::ExitProcess);

    }

    void RegisterAtExit(AtExitFunction fn, void *arg) {
        os::ScopedMutexLock lk(g_AtExitLock);
        g_AtExitEntries.PushBack({ fn, arg });
    }

    void SetExitFunction(ExitFunction fn) {
        g_ExitFunction = fn;
    }

    __attribute__((noreturn))
    void Exit(i32 error_code) {
        // Run all entries
        ProcessAtExitEntries();

        // g_ExitFunction must have a valid value, which is set by the CRT0
        g_ExitFunction(error_code);
        __builtin_unreachable();
    }

    extern "C" {

        // Compiler-generated atexit calls are called with this symbol.
        // TODO: make use of dso handle?

        i32 __cxa_atexit(AtExitFunction fn, void *arg, void *dso_handle) {
            RegisterAtExit(fn, arg);
            return 0;
        }
        
    }

}