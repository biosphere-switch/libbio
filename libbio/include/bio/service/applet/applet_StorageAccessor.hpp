
#pragma once
#include <bio/service/applet/applet_Types.hpp>
#include <bio/service/applet/applet_Results.hpp>
#include <bio/service/service_Types.hpp>

namespace bio::service::applet {

    class StorageAccessor : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetSize(u64 &out_size) {
                return this->session.SendRequestCommand<0>(ipc::client::Out(out_size));
            }

            inline Result Write(u64 offset, void *buf, u64 buf_size) {
                return this->session.SendRequestCommand<10>(ipc::client::In(offset), ipc::client::Buffer(buf, buf_size, ipc::BufferAttribute::In | ipc::BufferAttribute::AutoSelect));
            }

            inline Result Read(u64 offset, void *buf, u64 buf_size) {
                return this->session.SendRequestCommand<11>(ipc::client::In(offset), ipc::client::Buffer(buf, buf_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::AutoSelect));
            }

    };

}