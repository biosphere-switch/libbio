
#pragma once
#include <bio/service/lm/lm_Logger.hpp>

namespace bio::service::lm {

    class LogService : public ipc::client::Service {

        public:
            using Service::Service;

            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "lm";
            }

        public:
            inline Result OpenLogger(mem::SharedObject<Logger> &out_logger) {
                return this->session.SendRequestCommand<0>(ipc::client::InProcessId(), ipc::client::OutSessionObject<0, Logger>(out_logger));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(LogService)

}