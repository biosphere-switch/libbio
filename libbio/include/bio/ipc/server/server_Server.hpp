
#pragma once
#include <bio/ipc/server/server_Impl.hpp>

namespace bio::ipc::server {

    namespace impl {

        template<typename T>
        concept IsCommandArgument = requires(T t, CommandContext &ctx, CommandState state) {
            { t.Process(ctx, state) } -> util::SameAs<void>;
        };

        template<typename Arg>
        inline void ProcessCommandArgument(Arg &arg, CommandContext &ctx, CommandState state) {
            static_assert(IsCommandArgument<Arg>, "Invalid command argument");
            arg.Process(ctx, state);
        }

    }

    class Server {

        public:
            template<typename ...Args>
            void RequestCommandBegin(CommandContext &ctx, Args &&...args) {
                ctx.in.data_size = 0;
                (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeCommandHandler), ...);
            }

            template<typename ...Args>
            void RequestCommandEnd(CommandContext &ctx, Result rc, Args &&...args) {
                if(rc.IsSuccess()) {
                    (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeResponseWrite), ...);
                    WriteRequestCommandResponseOnTls(ctx, rc);
                    (impl::ProcessCommandArgument(args, ctx, CommandState::AfterResponseWrite), ...);
                }
                else {
                    WriteRequestCommandResponseOnTls(ctx, rc);
                }
            }

    };

    template<typename S>
    using CommandHandlerFunction = void(S::*)(CommandContext&);

    template<typename S>
    struct RequestCommandHandler {
        u32 request_id;
        // TODO: firmware version range
        CommandHandlerFunction<S> fn;
    };

    template<typename S>
    constexpr auto ServerCommandHandlers = S::template Handlers<S>; 

    template<typename S>
    constexpr auto ServerCommandHandleCount = sizeof(S::template Handlers<S>) / sizeof(RequestCommandHandler<S>);

};

#define BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS template<typename S> \
static constexpr ipc::server::RequestCommandHandler<S> Handlers[] =

#define BIO_IPC_SERVER_COMMAND_HANDLER(rq_id, name) { rq_id, &S::name }
