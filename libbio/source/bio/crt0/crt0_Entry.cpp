
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/hbl/hbl_HbAbi.hpp>

// Entrypoint

void Main();

namespace bio::crt0 {

    extern ExitFunction g_ExitFunction;

    // Will be used if hbl doesn't give us a heap

    __attribute__((weak))
    u64 g_HeapSize = 0x20000000;

    // Similar to rtld's functionality

    namespace {

        // Note: placing this as a separate function - if Main() or any calls inside the Entry function exited, this shared pointer wouldn't release properly.
        void RegisterBaseModule(void *aslr_base_address) {
            mem::SharedObject<dyn::Module> mod;
            dyn::LoadRawModule(aslr_base_address, mod);
        }

        void SetExitFunction(crt0::ExitFunction exit_lr) {
            if(exit_lr != nullptr) {
                g_ExitFunction = exit_lr;
            }
        }

    }

    __attribute__((weak))
    void Entry(void *context_args_ptr, u64 main_thread_handle_v, void *aslr_base_address, crt0::ExitFunction exit_lr) {
        DEBUG_LOG("Ohayo");

        // relocate ourselves
        dyn::RelocateModule(aslr_base_address);

        auto main_thread_handle = static_cast<u32>(main_thread_handle_v);

        // Set exit function (svc::ExitProcess if null)
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
            svc::SetHeapSize(heap_address, g_HeapSize);
        }

        // Initialize memory allocator
        mem::Initialize(heap_address, g_HeapSize);

        // Register self as a module (for init and fini arrays, etc)
        RegisterBaseModule(aslr_base_address);

        // Call entrypoint
        Main();

        // Successful exit by default
        Exit(SuccessExit);
    }

}
