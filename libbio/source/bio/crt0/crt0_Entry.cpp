
#include <bio/crt0/crt0_Types.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/os/os_Version.hpp>
#include <bio/hbl/hbl_HbAbi.hpp>
#include <bio/mem/mem_VirtualMemory.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>

// Entrypoint
// TODO: extern "C", multiple entrypoints...?

void Main();

namespace bio::os {

    extern Version g_SystemVersion;

    namespace {

        inline void SetSystemVersion(const Version &ver) {
            g_SystemVersion = ver;
        }

    }

}

namespace bio::mem {

    extern VirtualRegion g_AddressSpace;
    extern VirtualRegion g_StackRegion;
    extern VirtualRegion g_HeapRegion;
    extern VirtualRegion g_LegacyAliasRegion;

    namespace {

        Result ReadRegionInfo(svc::InfoId info_addr_id, svc::InfoId info_size_id, VirtualRegion &region) {
            u64 address;
            BIO_RES_TRY(svc::GetInfo(address, info_addr_id, svc::CurrentProcessPseudoHandle, 0));

            u64 size;
            BIO_RES_TRY(svc::GetInfo(size, info_size_id, svc::CurrentProcessPseudoHandle, 0));

            region.start = address;
            region.end = address + size;
            return ResultSuccess;            
        }

        void InitializeVirtualMemory() {
            auto rc = ReadRegionInfo(svc::InfoId::AslrRegionAddress, svc::InfoId::AslrRegionSize, g_AddressSpace);
            if(rc.IsFailure()) {
                // TODO: 1.0.0 support
            }
            BIO_DIAG_RES_ASSERT(ReadRegionInfo(svc::InfoId::StackRegionAddress, svc::InfoId::StackRegionSize, g_StackRegion));
            BIO_DIAG_RES_ASSERT(ReadRegionInfo(svc::InfoId::HeapRegionAddress, svc::InfoId::HeapRegionSize, g_HeapRegion));
            BIO_DIAG_RES_ASSERT(ReadRegionInfo(svc::InfoId::AliasRegionAddress, svc::InfoId::AliasRegionSize, g_LegacyAliasRegion));
        }

    }

}

namespace bio::crt0 {

    // Can be used to handle exceptions
    void ExceptionHandler(ExceptionDescription desc);

    // Must be implemented by the application
    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_size);

    namespace {

        os::Thread g_MainThread;

        // Note: since Entry() call will never exit the module object will never dispose, thus it wouldn't be properly cleaned up (AtExit would be called before it's even cleaned)
        void RegisterBaseModule(void *aslr_base_address) {
            mem::SharedObject<dyn::Module> mod;
            BIO_DIAG_RES_ASSERT(dyn::LoadRawModule(aslr_base_address, mod));
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

            // Initialize the thread object with a default name and its handle, and set it on the TLS
            g_MainThread.InitializeWith(main_thread_handle, "MainThread", reinterpret_cast<void*>(info.address), info.size, false);
            tls->thread_addr = &g_MainThread;
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

        inline void HandleException(ExceptionDescription error_desc) {
            svc::ReturnFromException(os::result::ResultUnhandledException);
            while(true) {}
        }

    }

    // Similar to offical rtld functionality - possible entries:

    // - When re-launched due to exception handling, x0 == exception description (error type) and x1 == stack top

    __attribute__((weak))
    void ExceptionEntry(ExceptionDescription error_desc, void *stack_top) {
        HandleException(error_desc);
    }

    // - When launched from hbloader/hbmenu (as a NRO), x0 == hbl config entries and x1 == -1
    // - When launched as a normal process (as a NSO), x0 == nullptr, and x1 == main thread handle

    __attribute__((weak))
    void NormalEntry(void *context_ptr, u64 main_thread_handle_v, void *aslr_base_address, crt0::ExitFunction lr, void *bss_start, void *bss_end) {
        const auto is_hbl_nro = (context_ptr != nullptr) && (main_thread_handle_v == -1);
        auto main_thread_handle = static_cast<u32>(main_thread_handle_v);
        
        // Clear .bss section
        ClearBss(bss_start, bss_end);

        // Relocate ourselves
        dyn::RelocateModule(aslr_base_address);

        // Set exit function if we're given a valid one (svc::ExitProcess is used by default)
        if(lr != nullptr) {
            SetExitFunction(lr);
        }

        // Initialize virtual memory.
        mem::InitializeVirtualMemory();

        void *hbl_heap_address = nullptr;
        auto hbl_heap_size = 0;
        u32 hbl_hos_version = 0;

        if(is_hbl_nro) {
            auto arg = reinterpret_cast<hbl::ABIConfigEntry*>(context_ptr);
            while(true) {
                auto key = static_cast<hbl::ABIConfigKey>(arg->key);
                if(key == hbl::ABIConfigKey::EOL) {
                    break;
                }
                switch(key) {
                    case hbl::ABIConfigKey::OverrideHeap: {
                        hbl_heap_address = reinterpret_cast<void*>(arg->value[0]);
                        hbl_heap_size = arg->value[1];
                        break;
                    }
                    case hbl::ABIConfigKey::MainThreadHandle: {
                        main_thread_handle = static_cast<u32>(arg->value[0]);
                        break;
                    }
                    case hbl::ABIConfigKey::HosVersion: {
                        hbl_hos_version = static_cast<u32>(arg->value[0]);
                        break;
                    }
                    // TODO: support more entries
                    default:
                        break;
                }
                arg++;
            }
        }

        // Setup TLS and main thread context
        SetupTlsMainThread(main_thread_handle);

        // Prepare heap (we might need to avoid svc::SetHeapSize in some contexts, like sysmodules/processes using fake stack heaps, so force the application to do it itself)
        void *heap_address = nullptr;
        u64 heap_size = 0;
        BIO_DIAG_RES_ASSERT(InitializeHeap(hbl_heap_address, hbl_heap_size, heap_address, heap_size));

        // Ensure heap address and size are valid.
        BIO_DIAG_ASSERT(heap_address != nullptr);
        BIO_DIAG_ASSERT(heap_size > 0);

        // Initialize memory allocator.
        mem::Initialize(heap_address, heap_size);

        // Set system version.
        BIO_DIAG_RES_ASSERT(SetSystemVersion(hbl_hos_version));

        // Load self as a module (for init and fini arrays, etc)
        RegisterBaseModule(aslr_base_address);

        // Call code entrypoint.
        Main();

        // Here is where Nintendo/libnx/libtransistor/etc. would dispose global services, modules, etc.
        // No disposing is made here since everything which should be disposed is implemented as shared objects, which are auto-disposed on program exit.

        // Successful exit by default.
        Exit(ExitSuccess);
    }

}
