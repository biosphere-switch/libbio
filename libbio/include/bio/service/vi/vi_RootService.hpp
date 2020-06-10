
#pragma once
#include <bio/service/vi/vi_ApplicationDisplayService.hpp>

namespace bio::service::vi {

    enum class RootServiceType {
        Application,
        System,
        Manager,
    };

    class RootService : public ipc::client::Service {

        public:
            using Service::Service;

        protected:
            template<u32 RequestId>
            inline Result GetDisplayServiceImpl(bool is_privileged, mem::SharedObject<ApplicationDisplayService> &out_service) {
                return this->session.SendRequestCommand<RequestId>(ipc::client::In<u32>(static_cast<u32>(is_privileged)), ipc::client::OutSessionObject<0, ApplicationDisplayService>(out_service));
            }

    };

    // TODO (same for nv): proper way to handle different services with identical interfaces depending on process type? current system is fine, but a better system might be helpful

    class ApplicationRootService : public RootService {

        public:
            using RootService::RootService;

        public:
            static inline constexpr bool IsDomain = true;

            static inline constexpr const char *GetName() {
                return "vi:u";
            }

        public:
            inline Result GetDisplayService(bool is_privileged, mem::SharedObject<ApplicationDisplayService> &out_service) {
                return this->GetDisplayServiceImpl<0>(is_privileged, out_service);
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(ApplicationRootService);

    class SystemRootService : public RootService {

        public:
            using RootService::RootService;

        public:
            static inline constexpr bool IsDomain = true;

            static inline constexpr const char *GetName() {
                return "vi:s";
            }

        public:
            inline Result GetDisplayService(bool is_privileged, mem::SharedObject<ApplicationDisplayService> &out_service) {
                return this->GetDisplayServiceImpl<1>(is_privileged, out_service);
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(SystemRootService);

    class ManagerRootService : public RootService {

        public:
            using RootService::RootService;

        public:
            static inline constexpr bool IsDomain = true;

            static inline constexpr const char *GetName() {
                return "vi:m";
            }

        public:
            inline Result GetDisplayService(bool is_privileged, mem::SharedObject<ApplicationDisplayService> &out_service) {
                return this->GetDisplayServiceImpl<2>(is_privileged, out_service);
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(ManagerRootService);

}