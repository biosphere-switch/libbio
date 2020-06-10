
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/nv/nv_Results.hpp>

namespace bio::service::nv {

    enum class DrvServiceType {
        Application,
        Applet,
        System,
    };

    class DrvService : public ipc::client::Service {

        public:
            using Service::Service;

        public:
            inline Result Open(char *path, u64 path_len, u32 &out_fd, ErrorCode &out_err_code) {
                return this->session.SendRequestCommand<0>(ipc::client::Buffer(path, path_len, ipc::BufferAttribute::In | ipc::BufferAttribute::MapAlias), ipc::client::Out<u32>(out_fd), ipc::client::Out<ErrorCode>(out_err_code));
            }

            inline Result Ioctl(u32 fd, IoctlId ioctl_id, void *in_buf, u64 in_buf_size, void *out_buf, u64 out_buf_size, ErrorCode &out_err_code) {
                return this->session.SendRequestCommand<1>(ipc::client::In<u32>(fd), ipc::client::In<IoctlId>(ioctl_id), ipc::client::Buffer(in_buf, in_buf_size, ipc::BufferAttribute::In | ipc::BufferAttribute::AutoSelect), ipc::client::Buffer(out_buf, out_buf_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::AutoSelect), ipc::client::Out<ErrorCode>(out_err_code));
            }

            inline Result Close(u32 fd, ErrorCode &out_err_code) {
                return this->session.SendRequestCommand<2>(ipc::client::In<u32>(fd), ipc::client::Out<ErrorCode>(out_err_code));
            }

            inline Result Initialize(u32 transfer_mem_handle, u32 transfer_mem_size, ErrorCode &out_err_code) {
                return this->session.SendRequestCommand<3>(ipc::client::InHandle<ipc::HandleMode::Copy>(svc::CurrentProcessPseudoHandle), ipc::client::InHandle<ipc::HandleMode::Copy>(transfer_mem_handle), ipc::client::In<u32>(transfer_mem_size), ipc::client::Out<ErrorCode>(out_err_code));
            }

    };

    class ApplicationDrvService : public DrvService {

        public:
            using DrvService::DrvService;

        public:
            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "nvdrv";
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(ApplicationDrvService);

    class AppletDrvService : public DrvService {

        public:
            using DrvService::DrvService;

        public:
            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "nvdrv:a";
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(AppletDrvService);

    class SystemDrvService : public DrvService {

        public:
            using DrvService::DrvService;

        public:
            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "nvdrv:s";
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(SystemDrvService);

}