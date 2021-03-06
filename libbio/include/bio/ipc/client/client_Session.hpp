
#pragma once
#include <bio/ipc/client/client_CommandArguments.hpp>
#include <bio/mem/mem_SharedObject.hpp>
#include <bio/util/util_Concepts.hpp>

namespace bio::ipc::client {

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

    struct Session : public SessionBase {

        using SessionBase::SessionBase;

        template<u32 RequestId, typename ...Args>
        inline Result SendRequestCommand(Args &&...args) {
            CommandContext ctx(this->GetBase());

            (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeHeaderInitialization), ...);
            
            WriteRequestCommandOnIpcBuffer(ctx, RequestId, DomainCommandType::SendMessage);
            
            (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeRequest), ...);

            BIO_RES_TRY(svc::SendSyncRequest(handle));

            (impl::ProcessCommandArgument(args, ctx, CommandState::AfterRequest), ...);
            
            BIO_RES_TRY(ReadRequestCommandResponseFromIpcBuffer(ctx));

            (impl::ProcessCommandArgument(args, ctx, CommandState::AfterResponseParse), ...);

            return ResultSuccess;
        }

        template<ControlRequestId RequestId, typename ...Args>
        inline Result SendControlCommand(Args &&...args) {
            CommandContext ctx(this->GetBase());

            (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeHeaderInitialization), ...);
            
            WriteControlCommandOnIpcBuffer(ctx, RequestId);
            
            (impl::ProcessCommandArgument(args, ctx, CommandState::BeforeRequest), ...);

            BIO_RES_TRY(svc::SendSyncRequest(handle));

            (impl::ProcessCommandArgument(args, ctx, CommandState::AfterRequest), ...);
            
            BIO_RES_TRY(ReadControlCommandResponseFromIpcBuffer(ctx));

            (impl::ProcessCommandArgument(args, ctx, CommandState::AfterResponseParse), ...);

            return ResultSuccess;
        }

        inline constexpr SessionBase &GetBase() {
            return static_cast<SessionBase&>(*this);
        }

        inline Result ConvertCurrentObjectToDomain() {
            return this->SendControlCommand<ControlRequestId::ConvertCurrentObjectToDomain>(Out<u32>(this->object_id));
        }

        inline Result CloneCurrentObject(Session &out_clone_session) {
            u32 cloned_handle;
            BIO_RES_TRY(this->SendControlCommand<ControlRequestId::CloneCurrentObject>(OutHandle<HandleMode::Move>(cloned_handle)));

            out_clone_session = Session::CreateFromHandle(cloned_handle);
            return ResultSuccess;
        }

        inline void SetPointerBufferSize() {
            this->QueryPointerBufferSize(this->pointer_buffer_size);
        }

        inline Result QueryPointerBufferSize(u16 &out_size) {
            return this->SendControlCommand<ControlRequestId::QueryPointerBufferSize>(Out<u16>(out_size));
        }

        inline void Close() {
            if(this->IsValid()) {
                if(this->IsDomain()) {
                    CommandContext ctx(this->GetBase());
                    WriteRequestCommandOnIpcBuffer(ctx, NoRequestId, DomainCommandType::Close);
                    svc::SendSyncRequest(this->handle);
                }
                else if(this->owns_handle) {
                    CommandContext ctx(this->GetBase());
                    WriteCloseCommandOnIpcBuffer(ctx);
                    svc::SendSyncRequest(this->handle);
                }
                if(this->owns_handle) {
                    svc::CloseHandle(this->handle);
                }
                this->object_id = InvalidObjectId;
                this->handle = InvalidHandle;
                this->owns_handle = false;
            }
        }

        static inline constexpr Session CreateFromHandle(u32 handle) {
            return { handle };
        }

        static inline constexpr Session CreateDomainFromParent(SessionBase parent, u32 object_id) {
            return { parent.GetHandle(), object_id };
        }

    };

    class SessionObject {

        protected:
            Session session;

        public:
            SessionObject(Session session) : session(session) {}

            ~SessionObject() {
                this->session.Close();
            }

            inline Session &GetSession() {
                return this->session;
            }

    };
    
    template<typename T>
    concept IsSessionObject = requires(T t) {
        { t.GetSession() } -> util::SameAs<Session&>;
    };

    // Service

    class Service : public SessionObject {

        public:
            using SessionObject::SessionObject;

        // Sample implementation - these MUST be implemented from derived types
        // For the service name, a function is used since the name we want may vary depending on aspects such as firmware version, etc. which obviously aren't constexpr

        /*
        public:
            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "set:sys";
            }

            inline Result PostInitialize() {
                // Optional
            }
        */

    };

    template<typename T>
    concept IsService = IsSessionObject<T> && requires(T) {
        { T::IsDomain } -> util::SameAs<const bool>;
        { T::GetName() } -> util::SameAs<const char*>;
    };

    // Named port (like sm:)

    class NamedPort : public SessionObject {

        public:
            using SessionObject::SessionObject;

        // Sample implementation - these MUST be implemented from derived types

        /*
        public:
            static inline constexpr const char *Name = "sm:";

            inline Result PostInitialize() {
                // Optional
            }
        */

    };

    template<typename T>
    concept IsNamedPort = IsSessionObject<T> && requires(T) {
        { T::Name } -> util::SameAs<const char* const>;
    };

    template<typename T>
    concept IsValidSessionType = IsService<T> || IsNamedPort<T>;

    template<typename T>
    concept HasPostInitialize = IsValidSessionType<T> && requires(T t) {
        { t.PostInitialize() } -> util::SameAs<Result>;
    };

    namespace impl {

        template<typename C>
        inline Result CreateNamedPortSession(Session &out_session) {
            static_assert(IsNamedPort<C>, "Invalid input");
            u32 out_handle;
            auto rc = svc::ConnectToNamedPort(out_handle, C::Name);
            if(rc.IsSuccess()) {
                out_session = Session::CreateFromHandle(out_handle);    
            }
            return rc;
        }

        // Note: this is implemented in sm namespace
        // Ignoring "not defined inline function" warning (it's defined elsewhere in the end)

        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wundefined-inline"

        template<typename S>
        inline Result CreateServiceSession(Session &out_session);

        #pragma clang diagnostic pop

        template<typename S>
        inline Result CreateSession(Session &out_session) { 
            static_assert(IsValidSessionType<S>, "Invalid input");
            if constexpr(IsNamedPort<S>) {
                return impl::CreateNamedPortSession<S>(out_session);
            }
            else if constexpr(IsService<S>) {
                return impl::CreateServiceSession<S>(out_session);
            }
            else {
                return result::ResultInvalidInput;
            }
        }

    }
    
    template<typename S>
    inline Result CreateSessionObject(mem::SharedObject<S> &out_obj) {
        static_assert(IsValidSessionType<S>, "Invalid input");
        Session session;
        BIO_RES_TRY(impl::CreateSession<S>(session));
        
        mem::SharedObject<S> session_obj;
        BIO_RES_TRY(mem::NewShared(session_obj, session));
        if constexpr(HasPostInitialize<S>) {
            BIO_RES_TRY(session_obj->PostInitialize());
        }
        
        out_obj = util::Move(session_obj);
        return ResultSuccess;
    }

}