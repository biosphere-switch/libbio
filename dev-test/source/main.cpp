
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

extern "C" {

    int bruh() {
        return 99;
    }

}

Result Test() {
    BIO_DIAG_LOG("Hello from Biosphere!");

    BIO_RES_TRY(service::sm::Initialize());
    BIO_RES_TRY(service::ro::Initialize());

    mem::SharedObject<service::fsp::FileSystemService> fspsrv;
    BIO_RES_TRY(service::CreateService(fspsrv));

    {
        mem::SharedObject<service::fsp::FileSystem> sd_fs;
        BIO_RES_TRY(fspsrv->OpenSdCardFileSystem(sd_fs));

        {
            mem::SharedObject<service::fsp::File> file;
            char path[0x301];
            mem::ZeroArray(path);
            util::SPrintf(path, "%s", "/dev-test.nro");
            BIO_RES_TRY(sd_fs->OpenFile(path, sizeof(path), BIO_BITMASK(0), file));

            u64 size = 0;
            BIO_RES_TRY(file->GetSize(size));

            BIO_DIAG_LOGF("NRO file size: %ld", size);

            auto buf = mem::Allocate(size);

            BIO_DIAG_LOGF("Buf: %p", buf);

            u64 read = 0;
            BIO_RES_TRY(file->Read(0, buf, size, 0, read));

            const auto nro_size = dyn::nro::GetNroSize(buf);
            BIO_DIAG_LOGF("NRO size: %d", nro_size);

            reinterpret_cast<dyn::nro::Header*>(buf)->bss_size = 0x1000;

            mem::SharedObject<dyn::Module> nro;
            BIO_RES_TRY(dyn::LoadNroModule(buf, nro_size, false, nro));

            int(*bruh)() = nullptr;
            BIO_RES_TRY(nro->ResolveSymbol("bruh", bruh));
            BIO_DIAG_LOGF("Function: %p", bruh);

            if(bruh != nullptr) {
                auto val = bruh();
                BIO_DIAG_LOGF("Bruh output: %d", val);
            }

        }
    }

    BIO_DIAG_LOG("Logging done...");

    return ResultSuccess;
}

void Main() {

    /*
    auto rc = Test();
    BIO_DIAG_LOGF("Test: 0x%X", rc.GetValue());
    */
    BIO_DIAG_LOG("Main()");

}