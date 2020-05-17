#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/svc/svc_Impl.hpp>

namespace bio::crt0 {

    namespace {

        struct ExitEntry {
            void(*fn)(void*);
            void *args;

            inline void Run() {
                (this->fn)(this->args);
            }

        };

        constexpr u32 MaxExitEntries = 16;

        ExitEntry g_ExitEntryList[MaxExitEntries];
        u32 g_ExitEntryCount;

        void CallAtExit() {
            for(u32 i = 0; i < g_ExitEntryCount; i++) {
                g_ExitEntryList[i].Run();
            }
        }

    }

    extern "C" {

        i32 __cxa_atexit(void (*fn)(void*), void *args, void *dso_handle) {
            if(g_ExitEntryCount < MaxExitEntries) {
                g_ExitEntryList[g_ExitEntryCount].fn = fn;
                g_ExitEntryList[g_ExitEntryCount].args = args;
                g_ExitEntryCount++;
            }
            return 0;
        }
        
    }

    ExitFunction g_ExitFunction = reinterpret_cast<ExitFunction>(&svc::ExitProcess);

    void Exit(i32 error_code) {
        // Dispose executing atexit calls
        // CallAtExit();
        DEBUG_LOG_FMT("Count: %d", g_ExitEntryCount);

        // g_ExitFunction must have a valid value, which is set by the CRT0
        g_ExitFunction(error_code);
    }

}