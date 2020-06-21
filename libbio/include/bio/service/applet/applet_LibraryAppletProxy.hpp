
#pragma once
#include <bio/service/applet/applet_LibraryAppletCreator.hpp>

namespace bio::service::applet {

    class LibraryAppletProxy : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetLibraryAppletCreator(mem::SharedObject<LibraryAppletCreator> &out_creator) {
                return this->session.SendRequestCommand<11>(ipc::client::OutSessionObject(out_creator));
            }

    };

}