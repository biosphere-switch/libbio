
#pragma once
#include <bio/ipc/server/server_Server.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

namespace bio::ipc::server {

    using ShouldMitmFunction = bool(*)(const service::sm::MitmProcessInfo&);

    class MitmQueryServer : public ipc::server::Server {

        private:
            ShouldMitmFunction should_mitm_fn;

        public:
            MitmQueryServer(ShouldMitmFunction fn) : should_mitm_fn(fn) {}

            Result ShouldMitm(ipc::CommandContext &ctx) {
                service::sm::MitmProcessInfo info;
                this->RequestCommandBegin(ctx, ipc::server::In<service::sm::MitmProcessInfo>(info));

                auto should_mitm = this->should_mitm_fn(info);
                return this->RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out<bool>(should_mitm));
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(65000, ShouldMitm),
            };

    };

}