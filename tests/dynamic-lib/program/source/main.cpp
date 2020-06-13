
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/fs/fs_Api.hpp>
#include <bio/dyn/dyn_Module.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("dynamic-lib-program-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

void Main() {
    BIO_DIAG_LOG("Main()");

    BIO_DIAG_RES_ASSERT(service::fsp::FileSystemServiceSession.Initialize());
    BIO_DIAG_RES_ASSERT(fs::MountSdCard("sd"));

    {
        mem::SharedObject<fs::File> lib_nro;
        BIO_DIAG_RES_ASSERT(fs::OpenFile("sd:/test/library.nro", service::fsp::FileOpenMode::Read, lib_nro));

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

            u32(*lib_test_Sample)();
            BIO_DIAG_RES_ASSERT(lib_nro_module->ResolveSymbol("_ZN3lib4test6SampleEv", lib_test_Sample));

            auto val_ret = lib_test_Sample();
            BIO_DIAG_LOGF("lib::test::Sample() from external NRO -> %d", val_ret);
        }
    }
    BIO_DIAG_LOG("Done");
}