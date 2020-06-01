
#include <bio/crt0/crt0_Types.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/os/os_Version.hpp>
#include <bio/hbl/hbl_HbAbi.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>

// Entrypoint

void Main();

namespace bio::crt0 {

    extern ExitFunction g_ExitFunction;

    // Will be used if hbl doesn't give us a heap
    __attribute__((weak))
    u64 g_HeapSize = 0x20000000;

    // Can be used to handle exceptions
    __attribute__((weak))
    void ExceptionHandler(ExceptionDescription desc) {
        // TODO: any IPC here seems to fail - why?
        // By default
        svc::ReturnFromException(os::result::ResultUnhandledException);
        while(true) {}
    }

    __attribute__((weak))
    Result InitializeHeap(void *heap_address, u64 heap_size, void *&out_heap_address, u64 &out_size) {
        if(heap_address == nullptr) {
            BIO_RES_TRY(svc::SetHeapSize(out_heap_address, heap_size));
        }
        else {
            out_heap_address = heap_address;
        }
        out_size = heap_size;
        return ResultSuccess;
    }

    namespace {

        os::Thread g_MainThread;

        // Note: placing this as a separate function - if Main() or any calls inside the Entry function exited, this shared pointer wouldn't release properly.
        void RegisterBaseModule(void *aslr_base_address) {
            mem::SharedObject<dyn::Module> mod;
            BIO_DIAG_RES_ASSERT(dyn::LoadRawModule(aslr_base_address, mod));
        }

        void SetExitFunction(crt0::ExitFunction lr) {
            if(lr != nullptr) {
                g_ExitFunction = lr;
            }
        }

        void ClearBss(void *bss_start, void *bss_end) {
            const auto bss_size = reinterpret_cast<u64>(bss_end) - reinterpret_cast<u64>(bss_start);
            mem::Zero(bss_start, bss_size);
        }

        void SetupTlsMainThread(u32 main_thread_handle) {
            auto tls = os::GetThreadLocalStorage();
            mem::ZeroSingle(tls);

            // Get the stack memory region.
            svc::MemoryInfo info;
            u32 page_info;
            BIO_DIAG_RES_ASSERT(svc::QueryMemory(info, page_info, reinterpret_cast<u64>(&info)));

            BIO_DIAG_RES_ASSERT(g_MainThread.InitializeWith(main_thread_handle, "MainThread", reinterpret_cast<void*>(info.address), info.size, false));
            tls->thread_ref = &g_MainThread;
        }

        Result SetSystemVersion(u32 hbl_hos_version) {
            if(hbl_hos_version > 0) {
                // Hbl gave us a system version, so use it.
                const auto major = static_cast<u8>((hbl_hos_version >> 16) & 0xFF);
                const auto minor = static_cast<u8>((hbl_hos_version >> 8) & 0xFF);
                const auto micro = static_cast<u8>(hbl_hos_version & 0xFF);
                os::SetSystemVersion({ major, minor, micro });
            }
            else {
                // Get system version from set:sys.
                service::ScopedSessionGuard setsys(service::set::sys::SysServiceSession);
                BIO_RES_TRY(setsys);

                service::set::sys::FirmwareVersion fwver;
                BIO_RES_TRY(service::set::sys::SysServiceSession->GetFirmwareVersion(fwver));

                os::SetSystemVersion({ fwver.major, fwver.minor, fwver.micro });
            }
            return ResultSuccess;
        }

    }

    // Similar to offical rtld functionality
    // x0 and x1 might be different things, depending on the context (data from NRO, exception handling data...)

    // When launched from hbloader (as a NRO), x0 == hbl config entries and x1 == -1
    // When launched as a normal process (NSO), x0 == nullptr, and x1 == main thread handle
    // When re-launched due to exception handling, x0 == exception description (error type) and x1 == stack top

    __attribute__((weak))
    void Entry(void *x0_v, u64 x1_v, void *aslr_base_address, crt0::ExitFunction lr, void *bss_start, void *bss_end) {
        // Clear .bss section
        ClearBss(bss_start, bss_end);

        // Relocate ourselves
        dyn::RelocateModule(aslr_base_address);
        
        const auto has_exception = (x0_v != nullptr) && (x1_v != -1);
        const auto is_hbl_nro = (x0_v != nullptr) && (x1_v == -1);

        if(has_exception) {
            auto desc = static_cast<ExceptionDescription>(reinterpret_cast<u64>(x0_v));
            ExceptionHandler(desc);
        }

        auto main_thread_handle = static_cast<u32>(x1_v);

        // Set exit function (svc::ExitProcess is used by default)
        SetExitFunction(lr);

        void *heap_address = nullptr;
        u32 hbl_hos_version = 0;

        if(is_hbl_nro) {
            auto arg = reinterpret_cast<hbl::ABIConfigEntry*>(x0_v);
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
                    case hbl::ABIConfigKey::HosVersion: {
                        hbl_hos_version = static_cast<u32>(arg->value[0]);
                        break;
                    };
                    default:
                        break;
                }
                arg++;
            }
        }

        // Prepare heap via this weak function (we might need to avoid svc::SetHeapSize in some contexts)
        void *actual_heap_address;
        u64 actual_heap_size;
        BIO_DIAG_RES_ASSERT(InitializeHeap(heap_address, g_HeapSize, actual_heap_address, actual_heap_size));

        // Prepare TLS and main thread context
        SetupTlsMainThread(main_thread_handle);

        // Initialize memory allocator.
        mem::Initialize(actual_heap_address, actual_heap_size);

        // Set system version.
        BIO_DIAG_RES_ASSERT(SetSystemVersion(hbl_hos_version));

        // Load self as a module (for init and fini arrays, etc)
        RegisterBaseModule(aslr_base_address);

        // Call code entrypoint.
        Main();

        // Note: no disposing is made here since everything which should be disposed is implemented as shared objects, which are auto-disposed on program exit.

        // Successful exit by default.
        Exit(SuccessExit);
    }

}
