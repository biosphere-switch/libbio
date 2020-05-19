
#pragma once
#include <bio/service/fsp/fsp_File.hpp>

namespace bio::service::fsp {

    class FileSystem : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateFile(char *path, u64 path_len, u32 flags, u64 size) {
                return this->session.SendRequestCommand<0>(ipc::client::In<u32>(flags), ipc::client::In<u64>(size), ipc::client::Buffer(path, path_len, ipc::client::BufferAttribute::In | ipc::client::BufferAttribute::Pointer));
            }
            
            inline Result OpenFile(char *path, u64 path_len, u32 flags, mem::SharedObject<File> &out_file) {
                return this->session.SendRequestCommand<8>(ipc::client::In<u32>(flags), ipc::client::Buffer(path, path_len, ipc::client::BufferAttribute::In | ipc::client::BufferAttribute::Pointer), ipc::client::OutSessionObject<0, File>(out_file));
            }

    };

}