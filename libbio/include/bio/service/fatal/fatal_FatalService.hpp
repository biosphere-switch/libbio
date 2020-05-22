
#pragma once
#include <bio/service/fatal/fatal_Types.hpp>

namespace bio::service::fatal {

    class FatalService : public ipc::client::Service {

        public:
            using Service::Service;

            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "fatal:u";
            }

        public:
            inline Result ThrowWithPolicy(Result rc, Policy mode) {
                return this->session.SendRequestCommand<1>(ipc::client::InProcessId(), ipc::client::In<u32>(rc.GetValue()), ipc::client::In<u32>(static_cast<u32>(mode)));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(FatalService);

}