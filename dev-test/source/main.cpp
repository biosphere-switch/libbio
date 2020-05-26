
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/util/util_List.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

#include <bio/ipc/server/server_ServerManager.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

namespace server {

    class TestService : public ipc::server::Server {

        public:
            void Demo0(ipc::CommandContext &ctx) {
                u64 val;
                this->RequestCommandBegin(ctx, ipc::server::In<u64>(val));

                BIO_DIAG_LOGF("Command 0 called -> value: %ld", val);

                if(val == 0) {
                    return this->RequestCommandEnd(ctx, 0xEBBE);
                }
                else {
                    return this->RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out<u32>(val * val));
                }
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(0, Demo0),
            };

    };

}

namespace client {

    class TestService : public ipc::client::Service {

        public:
            using ipc::client::Service::Service;

            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "bio-srv";
            }

        public:
            inline Result Demo0(u64 in, u32 &out) {
                return this->session.SendRequestCommand<0>(ipc::client::In<u64>(in), ipc::client::Out<u32>(out));
            }

    };

}

void clientThread(void*) {
    svc::SleepThread(500'000'000);
    BIO_DIAG_LOG("Starting client thread...");

    mem::SharedObject<client::TestService> srv;
    service::CreateService(srv);

    u32 out = 0;
    auto rc = srv->Demo0(9, out);

    BIO_DIAG_LOGF("bio-srv Demo0 -> rc: 0x%X, val: %d", rc.GetValue(), out);
}

namespace {

    ipc::server::ServerManager<1> man;

}

void IpcServerMain() {
    BIO_DIAG_LOG("IpcServerMain()");

    mem::SharedObject<os::ThreadObject> client;
    BIO_DIAG_RES_ASSERT(os::ThreadObject::Create(&clientThread, nullptr, 0x2000, "ClientThread", client));
    BIO_DIAG_RES_ASSERT(client->Start());

    BIO_DIAG_RES_ASSERT(man.RegisterServiceServer<server::TestService>(service::sm::ServiceName::Encode("bio-srv"), 0x10));

    BIO_DIAG_LOG("Looping process...");
    man.LoopProcess();

    BIO_DIAG_LOG("WTF?");
}

void Main() {
    BIO_DIAG_LOG("Main()");
    ((void(*)())0xCAFEBEEF0000BABE)();
    BIO_DIAG_LOG("After crash");
}