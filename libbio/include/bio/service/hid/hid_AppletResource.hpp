
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::hid {

    class AppletResource : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetSharedMemoryHandle(u32 &out_handle) {
                return this->session.SendRequestCommand<0>(ipc::client::OutHandle<ipc::HandleMode::Copy>(out_handle));
            }

    };

}