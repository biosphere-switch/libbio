
#pragma once
#include <bio/ipc/server/server_Server.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

namespace bio::ipc::server {

    using ShouldMitmFunction = bool(*)(const service::sm::MitmProcessInfo&);

    template<ShouldMitmFunction ShouldMitmFn>
    class MitmQueryServer : public ipc::server::Server {

        public:
            void ShouldMitm(ipc::CommandContext &ctx) {
                service::sm::MitmProcessInfo info;
                RequestCommandBegin(ctx, ipc::server::In<service::sm::MitmProcessInfo>(info));

                auto should_mitm = ShouldMitmFn(info);
                RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out<bool>(should_mitm));
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(65000, ShouldMitm),
            };

    };

}