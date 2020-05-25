
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::set {

    namespace sys {

        struct FirmwareVersion {
            u8 major;
            u8 minor;
            u8 micro;
            u8 padding1;
            u8 revision_major;
            u8 revision_minor;
            u8 padding2;
            u8 padding3;
            char platform[0x20];
            char version_hash[0x40];
            char display_version[0x18];
            char display_title[0x80];
        };
        static_assert(sizeof(FirmwareVersion) == 0x100, "FirmwareVersion");

        class SysService : public ipc::client::Service {

            public:
                using Service::Service;

                static inline constexpr bool IsDomain = false;

                static inline constexpr const char *GetName() {
                    return "set:sys";
                }

            public:
                inline Result GetFirmwareVersion(FirmwareVersion &out_version) {
                    return this->session.SendRequestCommand<3>(ipc::client::Buffer(&out_version, sizeof(out_version), ipc::BufferAttribute::Out | ipc::BufferAttribute::Pointer | ipc::BufferAttribute::FixedSize));
                }

        };

        BIO_SERVICE_DECLARE_GLOBAL_SESSION(SysService);

    }

}