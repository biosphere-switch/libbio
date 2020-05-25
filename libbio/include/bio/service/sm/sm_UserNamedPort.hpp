
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/sm/sm_Results.hpp>

namespace bio::service::sm {

    union ServiceName {

        static constexpr u64 NameLength = 8;

        char name[NameLength];
        u64 value;

        constexpr ServiceName() : value(0) {}

        inline constexpr u64 GetValue() {
            return this->value;
        }

        inline constexpr bool IsValid() {
            return this->value > 0;
        }

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

    };
    static_assert(sizeof(ServiceName) == ServiceName::NameLength, "Invalid");

    static inline constexpr ServiceName InvalidServiceName = {};

    class UserNamedPort : public ipc::client::NamedPort {

        private:
            inline bool CheckCanAccessServices() {
                ServiceName empty_srv_name = {};
                ipc::client::Session dummy_srv;
                auto rc = this->GetService(empty_srv_name, dummy_srv);
                return rc != result::ResultNotInitialized;
            }

        public:
            using NamedPort::NamedPort;

            static inline constexpr const char *Name = "sm:";

            inline Result PostInitialize() {
                return this->Initialize();
            }

        public:
            inline Result Initialize() {
                // If we can already access services, avoid calling the command
                if(this->CheckCanAccessServices()) {
                    return ResultSuccess;
                }
                return this->session.SendRequestCommand<0>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>(0));
            }

            inline Result GetService(ServiceName name, ipc::client::Session &out_session) {
                return this->session.SendRequestCommand<1>(ipc::client::In<u64>(name.GetValue()), ipc::client::OutSession<0>(out_session));
            }

            inline Result RegisterService(ServiceName name, bool is_light, i32 max_sessions, u32 &out_handle) {
                return this->session.SendRequestCommand<2>(ipc::client::In<u64>(name.GetValue()), ipc::client::In<bool>(is_light), ipc::client::In<i32>(max_sessions), ipc::client::OutHandle<ipc::HandleMode::Move, 0>(out_handle));
            }

            inline Result AtmosphereHasService(ServiceName name, bool &out_has) {
                return this->session.SendRequestCommand<65100>(ipc::client::In<u64>(name.GetValue()), ipc::client::Out<bool>(out_has));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(UserNamedPort);

}

namespace bio::ipc::client::impl {

    // Service creation implementation

    template<typename S>
    inline Result CreateServiceSession(Session &out_session) {
        static_assert(ipc::client::IsService<S>, "Invalid input");
        BIO_SERVICE_DO_WITH(sm, _sm_rc, {
            BIO_RES_TRY(_sm_rc);
            const auto name = S::GetName();
            DEBUG_LOG_FMT("sm - Name: '%s'", name);
            const auto srv_name = service::sm::ServiceName::Encode(name);
            Session tmp_session;
            BIO_RES_TRY(service::sm::UserNamedPortSession->GetService(srv_name, tmp_session));
            if(S::IsDomain) {
                BIO_RES_TRY(tmp_session.ConvertToDomain());
            }

            out_session = tmp_session;
            return ResultSuccess;
        });
        return 0xdead;
    }

}
