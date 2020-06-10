#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("acc-u0-mitm-client-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::ProcessExit;

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

void Main() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<AccService> acc;
    BIO_DIAG_RES_ASSERT(service::CreateService(acc));

    u32 count;
    BIO_DIAG_RES_ASSERT(acc->GetUserCount(count));

    BIO_DIAG_LOGF("Got user count: %d", count);

    BIO_DIAG_LOG("Done");
}