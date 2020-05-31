
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/util/util_List.hpp>
#include <bio/fs/fs_Api.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("dev-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

class AccService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "acc:u0";
        }

    public:
        inline Result GetUserCount(u32 &out_count) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_count));
        }

};

class BioDevService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "bio-dev";
        }

    public:
        inline Result Sample0(u32 &out_v) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_v));
        }

};

void DevMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<BioDevService> biodev;
    BIO_DIAG_RES_ASSERT(service::CreateService(biodev));

    u32 val;
    BIO_DIAG_RES_ASSERT(biodev->Sample0(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

void AccMitmMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<AccService> acc;
    BIO_DIAG_RES_ASSERT(service::CreateService(acc));

    u32 val;
    BIO_DIAG_RES_ASSERT(acc->GetUserCount(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

void Main() {
    BIO_DIAG_RES_ASSERT(service::fsp::FileSystemServiceSession.Initialize());
    BIO_DIAG_RES_ASSERT(fs::MountSdCard("sd"));
    BIO_DIAG_RES_ASSERT(fs::CreateFile("sd:/bio_fs_file.txt", 0, 0));
}
