
#pragma once
#include <bio/service/applet/applet_Storage.hpp>

namespace bio::service::applet {

    class LibraryAppletAccessor : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetAppletStateChangedEvent(u32 &out_event_handle) {
                return this->session.SendRequestCommand<0>(ipc::client::OutHandle<ipc::HandleMode::Copy>(out_event_handle));
            }

            inline Result Start() {
                return this->session.SendRequestCommand<10>();
            }

            inline Result PushInData(mem::SharedObject<Storage> &storage) {
                return this->session.SendRequestCommand<100>(ipc::client::InSessionObject(storage));
            }

    };

}