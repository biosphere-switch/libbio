
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/fsp/fsp_Types.hpp>
#include <bio/service/fsp/fsp_Results.hpp>

namespace bio::service::fsp {

    class File : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result Read(u64 offset, void *buf, u64 size, FileReadOption option, u64 &out_read) {
                return this->session.SendRequestCommand<0>(ipc::client::In<FileReadOption>(option), ipc::client::In<u32>(0), ipc::client::In<u64>(offset), ipc::client::In<u64>(size), ipc::client::Out<u64>(out_read), ipc::client::Buffer(buf, size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias | ipc::BufferAttribute::MapTransferAllowsNonSecure));
            }
            
            inline Result Write(u64 offset, void *buf, u64 size, FileWriteOption option) {
                return this->session.SendRequestCommand<1>(ipc::client::In<FileWriteOption>(option), ipc::client::In<u32>(0), ipc::client::In<u64>(offset), ipc::client::In<u64>(size), ipc::client::Buffer(buf, size, ipc::BufferAttribute::In | ipc::BufferAttribute::MapAlias | ipc::BufferAttribute::MapTransferAllowsNonSecure));
            }

            inline Result GetSize(u64 &out_size) {
                return this->session.SendRequestCommand<4>(ipc::client::Out<u64>(out_size));
            }

    };

}