#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("acc-u0-mitm-client-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

struct UserId {
    u64 high;
    u64 low;
};

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

        inline Result GetLastOpenedUser(UserId &out_user) {
            return this->session.SendRequestCommand<4>(ipc::client::Out<UserId>(out_user));
        }

};

void Main() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<AccService> acc;
    BIO_DIAG_RES_ASSERT(service::CreateService(acc));

    u32 count;
    BIO_DIAG_RES_ASSERT(acc->GetUserCount(count));

    BIO_DIAG_LOGF("Got user count: %d", count);

    UserId user;
    BIO_DIAG_RES_ASSERT(acc->GetLastOpenedUser(user));
    BIO_DIAG_LOGF("Last opened user { high: 0x%lX, low: 0x%lX }", user.high, user.low);

    BIO_DIAG_LOG("Done");
}