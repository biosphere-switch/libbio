
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/dyn/dyn_Relocation.hpp>
#include <bio/util/util_String.hpp>

#include <bio/crt0/crt0_ModuleName.hpp>

// Entrypoint

void Main();

namespace bio::crt0 {

    __attribute__((weak))
    __attribute__((section(".module_name")))
    volatile auto g_ModuleName = CRT0_MAKE_MODULE_NAME("libbio-nostd-weak");

    extern ExitFunction g_ExitFunction;

    // Similar to nn::init::Startup

    void Entry(void *context_args_ptr, u64 main_thread_handle_v, void *aslr_base_address, crt0::ExitFunction exit_lr) {
        auto main_thread_handle = static_cast<u32>(main_thread_handle_v);

        // TODO: implement lm logging
        DEBUG_LOG("Ohayo");

        dyn::RelocateModule(aslr_base_address);

        if(exit_lr != nullptr) {
            g_ExitFunction = exit_lr;
        }
        else {
            g_ExitFunction = reinterpret_cast<crt0::ExitFunction>(&svc::ExitProcess);
        }

        mem::Initialize(0x20000000);

        Main();

        Exit(SuccessExit);
    }

}
