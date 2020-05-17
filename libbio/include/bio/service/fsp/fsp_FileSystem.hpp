
#pragma once
#include <bio/service/fsp/fsp_File.hpp>

namespace bio::service::fsp {

    class FileSystem : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateFile(const char *path, u64 path_len, u32 flags, u64 size) {
                return this->session.SendSyncRequest<0>(ipc::client::In<u32>(flags), ipc::client::In<u64>(size), ipc::client::InStaticBuffer(path, path_len, 0));
            }
            
            inline Result OpenFile(const char *path, u64 path_len, u32 flags, mem::SharedObject<File> &out_file) {
                return this->session.SendSyncRequest<8>(ipc::client::In<u32>(flags), ipc::client::InStaticBuffer(path, path_len, 0), ipc::client::OutSessionObject<0, File>(out_file));
            }

    };

}