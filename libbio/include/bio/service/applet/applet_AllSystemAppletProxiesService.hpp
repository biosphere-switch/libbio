
#pragma once
#include <bio/service/applet/applet_LibraryAppletProxy.hpp>

namespace bio::service::applet {

    class AllSystemAppletProxiesService : public ipc::client::Service {

        public:
            using Service::Service;

            static inline constexpr bool IsDomain = true;

            static inline constexpr const char *GetName() {
                return "appletAE";
            }

        public:
            inline Result OpenLibraryAppletProxy(AppletAttribute attr, mem::SharedObject<LibraryAppletProxy> &out_proxy) {
                return this->session.SendRequestCommand<201>(ipc::client::InProcessId(), ipc::client::Buffer(&attr, sizeof(attr), ipc::BufferAttribute::In | ipc::BufferAttribute::MapAlias), ipc::client::InHandle<ipc::HandleMode::Copy>(svc::CurrentProcessPseudoHandle), ipc::client::OutSessionObject(out_proxy));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(AllSystemAppletProxiesService)

}