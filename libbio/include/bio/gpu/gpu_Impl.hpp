
#pragma once
#include <bio/gpu/gpu_Surface.hpp>
#include <bio/service/vi/vi_RootService.hpp>

namespace bio::gpu {

    constexpr i64 LayerMinimumZ = -1;
    constexpr i64 LayerMaximumZ = -2;

    Result Initialize(service::nv::DrvServiceType nv_service_type, service::vi::RootServiceType vi_service_type, u32 transfer_mem_size);
    void Finalize();

    Result GetHOSBinderDriver(mem::SharedObject<service::dispdrv::HOSBinderDriver> &out_service);
    Result GetNvDrvService(mem::SharedObject<service::nv::DrvService> &out_service);
    Result GetApplicationDisplayService(mem::SharedObject<service::vi::ApplicationDisplayService> &out_service);
    Result GetNvMapFd(u32 &out_fd);
    Result GetNvHostFd(u32 &out_fd);
    Result GetNvHostCtrlFd(u32 &out_fd);

    Result CreateStrayLayerSurface(const char *display_name, service::vi::LayerFlags layer_flags, u32 width, u32 height, u32 buffer_count, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, mem::SharedObject<Surface> &out_surface);
    Result CreateManagedLayerSurface(const char *display_name, service::vi::LayerFlags layer_flags, u64 aruid, f32 x, f32 y, u32 width, u32 height, i64 z, u32 buffer_count, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, mem::SharedObject<Surface> &out_surface);
}