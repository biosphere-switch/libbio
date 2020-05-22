
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/hbl/hbl_HbAbi.hpp>

#include <bio/diag/diag_Log.hpp>
#include <bio/service/service_Services.hpp>

// Entrypoint

void Main();

namespace bio::crt0 {

    extern ExitFunction g_ExitFunction;

    // Will be used if hbl doesn't give us a heap

    __attribute__((weak))
    u64 g_HeapSize = 0x20000000;

    // Similar to rtld's functionality

    namespace {

        os::Thread g_MainThread;

        // Note: placing this as a separate function - if Main() or any calls inside the Entry function exited, this shared pointer wouldn't release properly.
        void RegisterBaseModule(void *aslr_base_address) {
            mem::SharedObject<dyn::Module> mod;
            dyn::LoadRawModule(aslr_base_address, mod); // Assert
        }

        void SetExitFunction(crt0::ExitFunction exit_lr) {
            if(exit_lr != nullptr) {
                g_ExitFunction = exit_lr;
            }
        }

        void ClearBss(void *bss_start, void *bss_end) {
            const auto bss_size = reinterpret_cast<u64>(bss_end) - reinterpret_cast<u64>(bss_start);
            mem::Zero(bss_start, bss_size);
        }

        void SetupTlsMainThread(u32 main_thread_handle) {
            auto tls = os::GetThreadLocalStorage<os::ThreadLocalStorage>();
            mem::ZeroSingle(tls);

            // Get the stack memory region.
            svc::MemoryInfo info;
            u32 page_info;
            svc::QueryMemory(info, page_info, reinterpret_cast<u64>(&info)); // Assert

            g_MainThread.InitializeWith(main_thread_handle, "MainThreadDebug", reinterpret_cast<void*>(info.address), info.size, false); // Assert
            tls->thread_ref = &g_MainThread;
        }

    }

    // Similar to offical rtld functionality

    __attribute__((weak))
    void Entry(void *context_args_ptr, u64 main_thread_handle_v, void *aslr_base_address, crt0::ExitFunction exit_lr, void *bss_start, void *bss_end) {
        // Clear .bss section
        ClearBss(bss_start, bss_end);

        // Relocate ourselves
        dyn::RelocateModule(aslr_base_address);

        DEBUG_LOG("Ohayo");
        auto main_thread_handle = static_cast<u32>(main_thread_handle_v);

        // Set exit function (svc::ExitProcess is used by default)
        SetExitFunction(exit_lr);

        void *heap_address = nullptr;

        // Handle hbloader context if we were given it (aka if we were launched by hbl)
        if(context_args_ptr != nullptr) {
            auto arg = reinterpret_cast<hbl::ABIConfigEntry*>(context_args_ptr);
            while(true) {
                auto key = static_cast<hbl::ABIConfigKey>(arg->key);
                if(key == hbl::ABIConfigKey::EOL) {
                    break;
                }
                switch(key) {
                    case hbl::ABIConfigKey::OverrideHeap: {
                        heap_address = reinterpret_cast<void*>(arg->value[0]);
                        g_HeapSize = arg->value[1];
                        break;
                    }
                    case hbl::ABIConfigKey::MainThreadHandle: {
                        main_thread_handle = static_cast<u32>(arg->value[0]);
                        break;
                    }
                    default:
                        break;
                }
                arg++;
            }
        }

        if(heap_address == nullptr) {
            // We weren't given override heap - set it ourselves
            svc::SetHeapSize(heap_address, g_HeapSize); // Assert
        }

        // Initialize memory allocator
        mem::Initialize(heap_address, g_HeapSize);

        // Prepare TLS and main thread context
        SetupTlsMainThread(main_thread_handle);

        // Register self as a module (for init and fini arrays, etc)
        RegisterBaseModule(aslr_base_address);

        // Call entrypoint
        Main();

        // Note: no disposing is made here since everything which should be disposed is implemented as shared objects, which are auto-disposed on program exit.

        // Successful exit by default
        Exit(SuccessExit);
    }

}
