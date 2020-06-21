
#pragma once
#include <bio/service/applet/applet_StorageAccessor.hpp>

namespace bio::service::applet {

    class Storage : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result Open(mem::SharedObject<StorageAccessor> &out_accessor) {
                return this->session.SendRequestCommand<0>(ipc::client::OutSessionObject(out_accessor));
            }

    };

}