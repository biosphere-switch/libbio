
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
            Result RequestCommandEnd(CommandContext &ctx, Result rc, Args &&...args) {
                if(rc.IsSuccess()) {
                    (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeResponseWrite), ...);
                    WriteRequestCommandResponseOnIpcBuffer(ctx, rc);
                    (impl::ProcessCommandArgument(args, ctx, CommandState::AfterResponseWrite), ...);
                }
                else {
                    WriteRequestCommandResponseOnIpcBuffer(ctx, rc);
                }
                return rc;
            }

    };

    template<typename T, typename ...Args>
    concept IsServer = requires(T s, CommandContext &ctx, Result rc, Args &&...args) {
        { s.RequestCommandBegin(ctx, args...) } -> util::SameAs<void>;
        { s.RequestCommandEnd(ctx, rc, args...) } -> util::SameAs<Result>;
    };

    template<typename S>
    using CommandHandlerFunction = Result(S::*)(CommandContext&);

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

    // Service

    class Service : public Server {

        // Sample implementation - these MUST be implemented from derived types

        /*
        public:
            static inline constexpr const char *GetName() {
                return "srv";
            }

            static inline constexpr i32 GetMaxSessions() {
                return 0x10;
            }
        */

    };

    template<typename T>
    concept IsService = IsServer<T> && requires(T) {
        { T::GetName() } -> util::SameAs<const char*>;
        { T::GetMaxSessions() } -> util::SameAs<i32>;
    };

    class MitmService : public Server {

        // Sample implementation - these MUST be implemented from derived types

        /*
        public:
            static inline constexpr const char *GetName() {
                return "srv";
            }

            static inline constexpr bool ShouldMitm(const service::sm::MitmProcessInfo &info) {
                return true;
            }
        */

    };

    template<typename T>
    concept IsMitmService = IsServer<T> && requires(T, const service::sm::MitmProcessInfo &info) {
        { T::GetName() } -> util::SameAs<const char*>;
        { T::ShouldMitm(info) } -> util::SameAs<bool>;
    };

    // Named port (like sm:)

    class NamedPort : public Server {

        // Sample implementation - these MUST be implemented from derived types

        /*
        public:
            static inline constexpr const char *GetPortName() {
                return "port";
            }

            static inline constexpr i32 GetMaxSessions() {
                return 0x10;
            }
        */

    };

    template<typename T>
    concept IsNamedPort = IsServer<T> && requires(T) {
        { T::GetPortName() } -> util::SameAs<const char*>;
        { T::GetMaxSessions() } -> util::SameAs<i32>;
    };

};

#define BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS template<typename S> \
static constexpr ipc::server::RequestCommandHandler<S> Handlers[] =

#define BIO_IPC_SERVER_COMMAND_HANDLER(rq_id, name) { rq_id, &S::name }
