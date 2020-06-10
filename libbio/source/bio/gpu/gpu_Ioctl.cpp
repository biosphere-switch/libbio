#include <bio/gpu/gpu_Ioctl.hpp>
#include <bio/gpu/gpu_Impl.hpp>

namespace bio::gpu {

    Result DoIoctl(IoctlFd fd, service::nv::IoctlId ioctl_id, IoctlMode mode, void *data, u64 data_size) {
        mem::SharedObject<service::nv::DrvService> nvdrv;
        BIO_RES_TRY(GetNvDrvService(nvdrv));

        // TODO: support more fds.
        u32 fd_v = 0;
        switch(fd) {
            case IoctlFd::NvHost: {
                BIO_RES_TRY(GetNvHostFd(fd_v));
                break;
            }
            case IoctlFd::NvMap: {
                BIO_RES_TRY(GetNvMapFd(fd_v));
                break;
            }
            case IoctlFd::NvHostCtrl: {
                BIO_RES_TRY(GetNvHostCtrlFd(fd_v));
                break;
            }
        }
        BIO_RET_UNLESS(fd_v != 0, result::ResultInvalidIoctlFd);

        void *in_buf = nullptr;
        u64 in_buf_size = 0;
        if(static_cast<bool>(mode & IoctlMode::In)) {
            in_buf = data;
            in_buf_size = data_size;
        }
        void *out_buf = nullptr;
        u64 out_buf_size = 0;
        if(static_cast<bool>(mode & IoctlMode::Out)) {
            out_buf = data;
            out_buf_size = data_size;
        }
        
        service::nv::ErrorCode err;
        BIO_RES_TRY(nvdrv->Ioctl(fd_v, ioctl_id, in_buf, in_buf_size, out_buf, out_buf_size, err));
        BIO_RET_UNLESS(err == service::nv::ErrorCode::Success, service::nv::result::ConvertErrorCode(err));

        return ResultSuccess;
    }

}