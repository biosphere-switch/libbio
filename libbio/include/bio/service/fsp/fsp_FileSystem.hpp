
#pragma once
#include <bio/service/fsp/fsp_File.hpp>

namespace bio::service::fsp {

    class FileSystem : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateFile(char *path, u64 path_len, FileCreateOption option, u64 size) {
                return this->session.SendRequestCommand<0>(ipc::client::In<FileCreateOption>(option), ipc::client::In<u64>(size), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }
            
            inline Result OpenFile(char *path, u64 path_len, FileOpenMode mode, mem::SharedObject<File> &out_file) {
                return this->session.SendRequestCommand<8>(ipc::client::In<FileOpenMode>(mode), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer), ipc::client::OutSessionObject<0, File>(out_file));
            }

    };

}