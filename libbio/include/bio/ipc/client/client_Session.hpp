
#pragma once
#include <bio/ipc/client/client_RequestTypes.hpp>

namespace bio::ipc::client {

    struct RequestArgument;

    namespace impl {

        template<typename Arg>
        inline void ProcessRequestArgument(Arg &arg, RequestData &rq, RequestState state) {
            // static_assert(std::is_base_of_v<RequestArgument, Arg>, "Invalid request argument");
            arg.Process(rq, state);
        }

    }

    struct Session : public SessionBase {

        using SessionBase::SessionBase;

        template<u32 CommandId, typename ...Args>
        inline Result SendSyncRequest(Args &&...args) {
            auto rq = mem::Zeroed<RequestData>();
            rq.session_copy = *this;

            // Calculate in raw data size
            impl::OffsetCalculator off;
            auto offset_inmagic = off.GetNextOffset<u32>();
            off.IncrementOffset<u32>(); // u32 version
            auto offset_cmdid = off.GetNextOffset<u32>();
            off.IncrementOffset<u32>(); // u32 token
            rq.in_raw_size = off.GetCurrentOffset();

            (impl::ProcessRequestArgument(args, rq, RequestState::BeforeHeaderPreparation), ...);
            impl::PrepareCommandHeader(rq);

            // Fill in raw data now
            off = impl::OffsetCalculator(rq.in_raw);
            off.SetByOffset<u32>(offset_inmagic, SFCI);
            off.SetByOffset<u32>(offset_cmdid, CommandId);
            
            (impl::ProcessRequestArgument(args, rq, RequestState::BeforeRequest), ...);
            auto rc = svc::SendSyncRequest(handle);
            if(rc.IsSuccess()) {
                // Calculate out raw data size
                off = impl::OffsetCalculator();
                off.IncrementOffset<u32>(); // u32 magic (SFCO)
                off.IncrementOffset<u32>(); // u32 version
                auto offset_rc = off.GetNextOffset<u32>();
                off.IncrementOffset<u32>(); // u32 token
                rq.out_raw_size = off.GetCurrentOffset();

                (impl::ProcessRequestArgument(args, rq, RequestState::AfterRequest), ...);
                impl::ProcessResponse(rq);

                // Retrieve out raw data now (if the request succeeded)
                off = impl::OffsetCalculator(rq.out_raw);
                rc = off.GetByOffset<u32>(offset_rc);
                if(rc.IsSuccess()) {
                    (impl::ProcessRequestArgument(args, rq, RequestState::AfterResponseProcess), ...);
                }
            }
            return rc;
        }

        inline Result ConvertToDomain() {
            if(!this->IsDomain()) {
                return impl::ConvertObjectToDomain(this->handle, this->object_id);
            }
            return 0;
        }

        inline void Close() {
            if(this->IsValid()) {
                if(this->IsDomain()) {
                    impl::CloseDomainObject(this->handle, this->object_id);
                }
                else if(this->owns_handle) {
                    impl::CloseNonDomainObject(this->handle);
                }

                if(this->owns_handle) {
                    svc::CloseHandle(this->handle);
                }
            }
        }

        static inline Session CreateFromHandle(u32 handle) {
            u16 ptr_buf_size = 0;
            impl::QueryPointerBufferSize(handle, ptr_buf_size);
            return { handle, ptr_buf_size };
        }

        static inline constexpr Session CreateDomainFromParent(SessionBase parent, u32 object_id) {
            return { parent.GetHandle(), parent.GetPointerBufferSize(), object_id };
        }

    };

    class SessionObject {

        protected:
            Session session;

        public:
            SessionObject(Session session) : session(session) {
                // BIO_SVC_LOG_OUTPUT("Opened session of handle 0x%X", session.handle);
            }

            ~SessionObject() {
                // BIO_SVC_LOG_OUTPUT("Closing session of handle 0x%X", this->session.handle);
                this->session.Close();
            }

            inline Session &GetSession() {
                return this->session;
            }

            /*
            virtual Result PostInitialize() {
                return ResultSuccess;
            }
            */

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

            inline Result PostInitialize(Session &session) override {
                // Optional
            }
        */

    };

    // Named port (like sm:)

    class NamedPort : public SessionObject {

        public:
            using SessionObject::SessionObject;

        // Sample implementation - these MUST be implemented from derived types

        /*
        public:
            static inline constexpr const char Name[] = "sm:";

            inline Result PostInitialize(Session &session) override {
                // Optional
            }
        */

    };

    namespace impl {

        /*
        template<typename S>
        inline constexpr bool IsValidCreatableSession() {
            if constexpr(std::is_base_of_v<NamedPort, S>) {
                return true;
            }
            if constexpr(std::is_base_of_v<Service, S>) {
                return true;
            }
            return false;
        }
        */

        /*
        template<typename C>
        inline Result CreateNamedPortSession(Session &out_session) {
            // static_assert(std::is_base_of_v<NamedPort, C>, "Invalid input");
            u32 out_handle;
            auto rc = svc::ConnectToNamedPort(out_handle, C::Name);
            if(rc.IsSuccess()) {
                out_session = Session::CreateFromHandle(out_handle);    
            }
            return rc;
        }
        */

        // Note: this is implemented in sm namespace
        // Ignoring "not defined inline function" warning (it's defined elsewhere in the end)

        /*
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wundefined-inline"

        template<typename S>
        inline Result CreateServiceSession(Session &out_session);

        #pragma clang diagnostic pop
        */

        /*
        template<typename S>
        inline Result CreateSession(Session &out_session) { 
            static_assert(IsValidCreatableSession<S>(), "Invalid input");
            if constexpr(std::is_base_of_v<NamedPort, S>) {
                return impl::CreateNamedPortSession<S>(out_session);
            }
            else if constexpr(std::is_base_of_v<Service, S>) {
                return impl::CreateServiceSession<S>(out_session);
            }
            else {
                return result::ResultInvalidInput;
            }
        }
        */

    }
    
    /*
    template<typename S>
    inline Result CreateSessionObject(SharedPointer<S> &out_obj) {
        static_assert(std::is_base_of_v<SessionObject, S>, "Invalid input");
        Session session;
        BIO_R_TRY(impl::CreateSession<S>(session));
        
        auto session_obj = std::make_shared<S>(session);
        BIO_R_TRY(session_obj->PostInitialize());
        
        out_obj = std::move(session_obj);
        return ResultSuccess;
    }
    */

}