
#pragma once
#include <bio/service/fsp/fsp_File.hpp>
#include <bio/service/fsp/fsp_Directory.hpp>

namespace bio::service::fsp {

    class FileSystem : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateFile(char *path, u64 path_len, FileAttribute option, u64 size) {
                return this->session.SendRequestCommand<0>(ipc::client::In<FileAttribute>(option), ipc::client::In<u64>(size), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result DeleteFile(char *path, u64 path_len) {
                return this->session.SendRequestCommand<1>(ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result CreateDirectory(char *path, u64 path_len) {
                return this->session.SendRequestCommand<2>(ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result DeleteDirectory(char *path, u64 path_len) {
                return this->session.SendRequestCommand<3>(ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result DeleteDirectoryRecursively(char *path, u64 path_len) {
                return this->session.SendRequestCommand<4>(ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result GetEntryType(char *path, u64 path_len, DirectoryEntryType &out_type) {
                return this->session.SendRequestCommand<7>(ipc::client::Out<DirectoryEntryType>(out_type), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }
            
            inline Result OpenFile(char *path, u64 path_len, FileOpenMode mode, mem::SharedObject<File> &out_file) {
                return this->session.SendRequestCommand<8>(ipc::client::In<FileOpenMode>(mode), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer), ipc::client::OutSessionObject(out_file));
            }

            inline Result OpenDirectory(char *path, u64 path_len, DirectoryOpenMode mode, mem::SharedObject<Directory> &out_directory) {
                return this->session.SendRequestCommand<9>(ipc::client::In<DirectoryOpenMode>(mode), ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer), ipc::client::OutSessionObject(out_directory));
            }

    };

}