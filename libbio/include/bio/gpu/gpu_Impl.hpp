
#pragma once
#include <bio/gpu/gpu_Surface.hpp>
#include <bio/service/vi/vi_RootService.hpp>

namespace bio::gpu {

    Result Initialize(service::nv::DrvServiceType type, u32 transfer_mem_size);
    void Finalize();

    Result GetHOSBinderDriver(mem::SharedObject<service::dispdrv::HOSBinderDriver> &out_service);
    Result GetNvDrvService(mem::SharedObject<service::nv::DrvService> &out_service);
    Result GetApplicationDisplayService(mem::SharedObject<service::vi::ApplicationDisplayService> &out_service);
    Result GetNvMapFd(u32 &out_fd);
    Result GetNvHostFd(u32 &out_fd);
    Result GetNvHostCtrlFd(u32 &out_fd);

    Result CreateLayerSurface(const char *display_name, u64 aruid, u32 buffer_count, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, mem::SharedObject<Surface> &out_surface);
    Result CreateStrayLayerSurface(const char *display_name, service::vi::LayerFlags stray_layer_flags, u32 buffer_count, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, mem::SharedObject<Surface> &out_surface);

}