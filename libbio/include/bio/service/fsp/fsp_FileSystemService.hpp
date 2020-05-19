
#pragma once
#include <bio/service/fsp/fsp_FileSystem.hpp>

namespace bio::service::fsp {

    class FileSystemService : public ipc::client::Service {

        public:
            using Service::Service;

            static inline constexpr bool IsDomain = true;

            static inline constexpr const char *GetName() {
                return "fsp-srv";
            }

            inline Result PostInitialize() {
                return this->SetCurrentProcess();
            }

        public:
            inline Result SetCurrentProcess() {
                return this->session.SendRequestCommand<1>(ipc::client::InProcessId());
            }

            inline Result OpenSdCardFileSystem(mem::SharedObject<FileSystem> &out_fs) {
                return this->session.SendRequestCommand<18>(ipc::client::OutSessionObject<0, FileSystem>(out_fs));
            }

    };

}