
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

            /*
            inline Result PostInitialize() override {
                return this->SetCurrentProcess();
            }
            */

        public:
            inline Result SetCurrentProcess() {
                return this->session.SendSyncRequest<1>(ipc::client::InProcessId(), ipc::client::In<u64>(0));
            }

            inline Result OpenSdCardFileSystem(ipc::client::Session &out_fs) {
                return this->session.SendSyncRequest<18>(ipc::client::OutSession<0>(out_fs));
            }

    };

}