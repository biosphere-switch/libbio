
#pragma once
#include <bio/service/applet/applet_LibraryAppletAccessor.hpp>

namespace bio::service::applet {

    class LibraryAppletCreator : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateLibraryApplet(AppletId id, LibraryAppletMode mode, mem::SharedObject<LibraryAppletAccessor> &out_accessor) {
                return this->session.SendRequestCommand<0>(ipc::client::In(id), ipc::client::In(mode), ipc::client::OutSessionObject(out_accessor));
            }

            inline Result CreateStorage(u64 size, mem::SharedObject<Storage> &out_storage) {
                return this->session.SendRequestCommand<10>(ipc::client::In(size), ipc::client::OutSessionObject(out_storage));
            }

    };

}