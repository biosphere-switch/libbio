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

        constexpr u32 MaxExitEntries = 16;

        util::SizedArray<ExitEntry, MaxExitEntries> g_ExitEntries;

        void CallAtExit() {
            while(g_ExitEntries.Any()) {
                auto &entry = g_ExitEntries.Back();
                entry.Run();
                g_ExitEntries.Pop();
            }
        }

    }

    extern "C" {

        i32 __cxa_atexit(void (*fn)(void*), void *args, void *dso_handle) {
            // TODO: make use of dso handle?
            g_ExitEntries.Push({ fn, args });
            return 0;
        }
        
    }

    ExitFunction g_ExitFunction = reinterpret_cast<ExitFunction>(&svc::ExitProcess);

    void Exit(i32 error_code) {
        // Dispose executing atexit calls
        // CallAtExit();

        // g_ExitFunction must have a valid value, which is set by the CRT0
        g_ExitFunction(error_code);
    }

}