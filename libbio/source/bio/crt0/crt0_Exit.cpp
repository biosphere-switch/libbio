#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/util/util_Array.hpp>

namespace bio::crt0 {

    namespace {

        struct ExitEntry {
            void(*fn)(void*);
            void *args;

            inline void Run() {
                if(this->fn != nullptr) {
                    (this->fn)(this->args);
                }
            }

        };

        // TODO: switch to a dynamic array

        constexpr u32 MaxExitEntries = 32;

        util::SizedArray<ExitEntry, MaxExitEntries> g_ExitEntries;

        void ProcessAtExitEntries() {
            while(g_ExitEntries.Any()) {
                auto &entry = g_ExitEntries.Back();
                entry.Run();
                g_ExitEntries.Pop();
            }
        }

    }

    ExitFunction g_ExitFunction = reinterpret_cast<ExitFunction>(&svc::ExitProcess);

    void RegisterAtExit(AtExitFunction fn, void *arg) {
        g_ExitEntries.Push({ fn, arg });
    }

    void Exit(i32 error_code) {
        // Run all entries
        ProcessAtExitEntries();

        // g_ExitFunction must have a valid value, which is set by the CRT0
        g_ExitFunction(error_code);
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