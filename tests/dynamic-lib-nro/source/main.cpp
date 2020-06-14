
#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/fs/fs_Api.hpp>
#include <bio/dyn/dyn_Module.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("dynamic-lib-nro-test");

    constexpr u64 HeapSize = 128_MB;

    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_heap_size) {
        if(hbl_heap_address != nullptr) {
            out_heap_address = hbl_heap_address;
            out_heap_size = hbl_heap_size;
        }
        else {
            void *heap_addr;
            BIO_RES_TRY(svc::SetHeapSize(heap_addr, HeapSize));

            out_heap_address = heap_addr;
            out_heap_size = HeapSize;
        }
        return ResultSuccess;
    }

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

extern "C" {

    u32 dynamicLibTest() {
        return 69;
    }

}

namespace dynamic::lib {

    u32 CppNameTest() {
        return 420;
    }

}

void Main() {
    BIO_DIAG_LOG("Main()");

    BIO_DIAG_RES_ASSERT(service::fsp::FileSystemServiceSession.Initialize());
    BIO_DIAG_RES_ASSERT(fs::MountSdCard("sd"));

    {
        // Open ourselves
        mem::SharedObject<fs::File> lib_nro;
        BIO_DIAG_RES_ASSERT(fs::OpenFile("sd:/switch/dynamic-lib-nro.nro", service::fsp::FileOpenMode::Read, lib_nro));

        u64 f_size = 0;
        lib_nro->GetSize(f_size);
        if(f_size > 0) {
            void *lib_nro_buf;
            BIO_DIAG_RES_ASSERT(mem::Allocate<mem::PageAlignment>(f_size, lib_nro_buf));

            mem::Zero(lib_nro_buf, f_size);

            BIO_DIAG_RES_ASSERT(lib_nro->Read(lib_nro_buf, f_size));

            BIO_DIAG_RES_ASSERT(service::ro::RoServiceSession.Initialize());

            mem::SharedObject<dyn::Module> lib_nro_module;
            BIO_DIAG_RES_ASSERT(dyn::LoadNroModule(lib_nro_buf, false, lib_nro_module));

            u32(*__fn_dynamicLibTest)();
            BIO_DIAG_RES_ASSERT(lib_nro_module->ResolveSymbol("dynamicLibTest", __fn_dynamicLibTest));

            BIO_DIAG_ASSERT(__fn_dynamicLibTest() == dynamicLibTest());

            u32 (*__fn_dynamic_lib_CppNameTest)();
            BIO_DIAG_RES_ASSERT(lib_nro_module->ResolveSymbol("_ZN7dynamic3lib11CppNameTestEv", __fn_dynamic_lib_CppNameTest));

            BIO_DIAG_ASSERT(__fn_dynamic_lib_CppNameTest() == dynamic::lib::CppNameTest());

            BIO_DIAG_LOG("Done - test succeeded");
        }
    }
}