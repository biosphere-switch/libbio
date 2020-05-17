
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/sm/sm_Results.hpp>

namespace bio::service::sm {

    struct ServiceName {

        static constexpr u64 NameLength = 8;

        union {
            char name[NameLength];
            u64 value;
        };

        static inline constexpr ServiceName Encode(const char *name) {
            ServiceName srv_name = {};
            for(u64 i = 0; i < NameLength; i++) {
                const char chr = name[i];
                srv_name.name[i] = chr;
                if(chr == '\0') {
                    break;
                }
            }
            return srv_name;
        }

        inline constexpr u64 GetValue() {
            return this->value;
        }

    };
    static_assert(sizeof(ServiceName) == ServiceName::NameLength, "Invalid");

    class UserNamedPort : public ipc::client::NamedPort {

        private:
            inline bool CheckCanAccessServices() {
                ServiceName empty_service = {};
                u32 dummy_handle;
                auto rc = this->GetService(empty_service, dummy_handle);
                return rc != 0x415; // check that the error isn't "not initialized"
            }

        public:
            using NamedPort::NamedPort;

            static inline constexpr const char *Name = "sm:";

            inline Result PostInitialize() {
                return this->Initialize();
            }

        public:
            inline Result Initialize() {
                if(this->CheckCanAccessServices()) {
                    return ResultSuccess;
                }
                return this->session.SendSyncRequest<0>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>(0), ipc::client::In<u64>(0));
            }

            inline Result GetService(ServiceName name, u32 &out_handle) {
                return this->session.SendSyncRequest<1>(ipc::client::In<u64>(name.GetValue()), ipc::client::OutHandle<0>(out_handle));
            }

            inline Result AtmosphereHasService(ServiceName name, bool &out_has) {
                return this->session.SendSyncRequest<65100>(ipc::client::In<u64>(name.GetValue()), ipc::client::Out<bool>(out_has));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(UserNamedPort);

}

namespace bio::ipc::client::impl {

    // Service creation implementation

    template<typename S>
    inline Result CreateServiceSession(Session &out_session) {
        static_assert(ipc::client::IsService<S>, "Invalid input");
        if(service::sm::IsInitialized()) {
            const auto name = S::GetName();
            DEBUG_LOG_FMT("sm - Name: '%s'", name);
            const auto srv_name = service::sm::ServiceName::Encode(name);
            Session tmp_session;
            u32 srv_handle;
            auto rc = service::sm::UserNamedPortSession->GetService(srv_name, srv_handle);
            if(rc.IsSuccess()) {
                tmp_session = Session::CreateFromHandle(srv_handle);
                if(S::IsDomain) {
                    rc = tmp_session.ConvertToDomain();
                }
            }
            if(rc.IsSuccess()) {
                out_session = tmp_session;
            }
            return rc;
        }
        return 0xdead;
    }

}