
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::fsp {

    class File : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetSize(u64 &out_size) {
                return this->session.SendSyncRequest<4>(ipc::client::Out<u64>(out_size));
            }

            inline Result Read(u64 offset, void *buf, u64 size, u32 option, u64 &out_read) {
                return this->session.SendSyncRequest<0>(ipc::client::In<u32>(option), ipc::client::In<u32>(0), ipc::client::In<u64>(offset), ipc::client::In<u64>(size), ipc::client::Out<u64>(out_read), ipc::client::OutBuffer(buf, size, 1));
            }
            
            inline Result Write(u64 offset, const void *buf, u64 size, u32 option) {
                return this->session.SendSyncRequest<1>(ipc::client::In<u32>(option), ipc::client::In<u32>(0), ipc::client::In<u64>(offset), ipc::client::In<u64>(size), ipc::client::InBuffer(buf, size, 1));
            }

    };

}