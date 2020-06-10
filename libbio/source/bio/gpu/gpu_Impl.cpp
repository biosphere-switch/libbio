#include <bio/gpu/gpu_Impl.hpp>
#include <bio/gpu/gpu_Ioctl.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/crt0/crt0_Exit.hpp>

#include <bio/diag/diag_Log.hpp>

namespace bio::gpu {

    namespace {

        service::nv::DrvServiceType g_NvDrvServiceType;
        service::vi::RootServiceType g_ViRootServiceType;
        mem::SharedObject<service::vi::ApplicationDisplayService> g_ViApplicationDisplayService;
        mem::SharedObject<service::vi::SystemDisplayService> g_ViSystemDisplayService;
        mem::SharedObject<service::dispdrv::HOSBinderDriver> g_HOSBinderDriver;

        void *g_TransferMemory;
        u32 g_TransferMemorySize;
        u32 g_TransferMemoryHandle;
        bool g_Initialized = false;

        u32 g_NvHostFd;
        u32 g_NvMapFd;
        u32 g_NvHostCtrlFd;

        inline bool IsNvDrvServiceInitialized() {
            switch(g_NvDrvServiceType) {
                case service::nv::DrvServiceType::Application: {
                    return service::nv::ApplicationDrvServiceSession.IsInitialized();
                }
                case service::nv::DrvServiceType::Applet: {
                    return service::nv::AppletDrvServiceSession.IsInitialized();
                }
                case service::nv::DrvServiceType::System: {
                    return service::nv::SystemDrvServiceSession.IsInitialized();
                }
            }
        }

        inline mem::SharedObject<service::nv::DrvService> &GetNvDrvServiceSession() {
            switch(g_NvDrvServiceType) {
                case service::nv::DrvServiceType::Application: {
                    return reinterpret_cast<mem::SharedObject<service::nv::DrvService>&>(service::nv::ApplicationDrvServiceSession.Get());
                }
                case service::nv::DrvServiceType::Applet: {
                    return reinterpret_cast<mem::SharedObject<service::nv::DrvService>&>(service::nv::AppletDrvServiceSession.Get());
                }
                case service::nv::DrvServiceType::System: {
                    return reinterpret_cast<mem::SharedObject<service::nv::DrvService>&>(service::nv::SystemDrvServiceSession.Get());
                }
            }
        }

        inline Result InitializeNvDrvService(service::nv::DrvServiceType type) {
            switch(type) {
                case service::nv::DrvServiceType::Application: {
                    BIO_RES_TRY(service::nv::ApplicationDrvServiceSession.Initialize());
                }
                case service::nv::DrvServiceType::Applet: {
                    BIO_RES_TRY(service::nv::AppletDrvServiceSession.Initialize());
                }
                case service::nv::DrvServiceType::System: {
                    BIO_RES_TRY(service::nv::SystemDrvServiceSession.Initialize());
                }
            }
            return ResultSuccess;
        }

        inline bool IsViServiceInitialized() {
            switch(g_ViRootServiceType) {
                case service::vi::RootServiceType::Application: {
                    return service::vi::ApplicationRootServiceSession.IsInitialized();
                }
                case service::vi::RootServiceType::System: {
                    return service::vi::SystemRootServiceSession.IsInitialized();
                }
                case service::vi::RootServiceType::Manager: {
                    return service::vi::ManagerRootServiceSession.IsInitialized();
                }
            }
        }

        inline mem::SharedObject<service::vi::RootService> &GetViServiceSession() {
            switch(g_ViRootServiceType) {
                case service::vi::RootServiceType::Application: {
                    return reinterpret_cast<mem::SharedObject<service::vi::RootService>&>(service::vi::ApplicationRootServiceSession.Get());
                }
                case service::vi::RootServiceType::System: {
                    return reinterpret_cast<mem::SharedObject<service::vi::RootService>&>(service::vi::SystemRootServiceSession.Get());
                }
                case service::vi::RootServiceType::Manager: {
                    return reinterpret_cast<mem::SharedObject<service::vi::RootService>&>(service::vi::ManagerRootServiceSession.Get());
                }
            }
        }

        inline Result InitializeViService() {
            switch(g_NvDrvServiceType) {
                case service::nv::DrvServiceType::Application: {
                    BIO_RES_TRY(service::vi::ApplicationRootServiceSession.Initialize());
                    g_ViRootServiceType = service::vi::RootServiceType::Application;
                    break;
                }
                case service::nv::DrvServiceType::Applet: {
                    BIO_RES_TRY(service::vi::SystemRootServiceSession.Initialize());
                    g_ViRootServiceType = service::vi::RootServiceType::System;
                    break;
                }
                case service::nv::DrvServiceType::System: {
                    BIO_RES_TRY(service::vi::ManagerRootServiceSession.Initialize());
                    g_ViRootServiceType = service::vi::RootServiceType::Manager;
                    break;
                }
            }
            return ResultSuccess;
        }

        inline Result ViRootServiceGetDisplayService(bool is_privileged, mem::SharedObject<service::vi::ApplicationDisplayService> &out_service) {
            switch(g_ViRootServiceType) {
                case service::vi::RootServiceType::Application: {
                    BIO_RES_TRY(service::vi::ApplicationRootServiceSession->GetDisplayService(is_privileged, out_service));
                    break;
                }
                case service::vi::RootServiceType::System: {
                    BIO_RES_TRY(service::vi::SystemRootServiceSession->GetDisplayService(is_privileged, out_service));
                    break;
                }
                case service::vi::RootServiceType::Manager: {
                    BIO_RES_TRY(service::vi::ManagerRootServiceSession->GetDisplayService(is_privileged, out_service));
                    break;
                }
            }
            return ResultSuccess;
        }

        Result CreateSurfaceImpl(u32 buffer_count, u64 display_id, u64 layer_id, bool is_stray_layer, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, ParcelPayload &native_window_payload, mem::SharedObject<Surface> &out_surface) {
            Parcel parcel;
            BIO_RES_TRY(parcel.LoadFrom(native_window_payload));

            ParcelData data;
            BIO_RES_TRY(parcel.Read(data));

            BIO_RES_TRY(g_HOSBinderDriver->AdjustRefcount(data.handle, 1, service::dispdrv::RefcountType::Weak));
            BIO_RES_TRY(g_HOSBinderDriver->AdjustRefcount(data.handle, 1, service::dispdrv::RefcountType::Strong));

            mem::SharedObject<Surface> surface;
            BIO_RES_TRY(mem::NewShared(surface, data.handle, buffer_count, display_id, layer_id, is_stray_layer, color_fmt, pixel_fmt, layout));

            BIO_RES_TRY(surface->InitializeAll());

            out_surface = util::Move(surface);
            return ResultSuccess;
        }

    }

    Result Initialize(service::nv::DrvServiceType type, u32 transfer_mem_size) {
        BIO_RET_UNLESS(!g_Initialized, ResultSuccess);

        BIO_RES_TRY(InitializeNvDrvService(type));

        BIO_RES_TRY(mem::Allocate<mem::PageAlignment>(transfer_mem_size, g_TransferMemory));

        g_NvDrvServiceType = type;
        g_TransferMemorySize = transfer_mem_size;

        auto &nvdrv = GetNvDrvServiceSession();

        BIO_RES_TRY(svc::CreateTransferMemory(g_TransferMemoryHandle, g_TransferMemory, g_TransferMemorySize, svc::MemoryPermission::None));

        service::nv::ErrorCode err;
        BIO_RES_TRY(nvdrv->Initialize(g_TransferMemoryHandle, g_TransferMemorySize, err));
        BIO_RET_UNLESS(err == service::nv::ErrorCode::Success, service::nv::result::ConvertErrorCode(err));

        auto dev = "/dev/nvhost-as-gpu";
        BIO_RES_TRY(nvdrv->Open(const_cast<char*>(dev), BIO_UTIL_STRLEN(dev), g_NvHostFd, err));
        BIO_RET_UNLESS(err == service::nv::ErrorCode::Success, service::nv::result::ConvertErrorCode(err));

        dev = "/dev/nvmap";
        BIO_RES_TRY(nvdrv->Open(const_cast<char*>(dev), BIO_UTIL_STRLEN(dev), g_NvMapFd, err));
        BIO_RET_UNLESS(err == service::nv::ErrorCode::Success, service::nv::result::ConvertErrorCode(err));

        dev = "/dev/nvhost-ctrl";
        BIO_RES_TRY(nvdrv->Open(const_cast<char*>(dev), BIO_UTIL_STRLEN(dev), g_NvHostCtrlFd, err));
        BIO_RET_UNLESS(err == service::nv::ErrorCode::Success, service::nv::result::ConvertErrorCode(err));

        BIO_RES_TRY(InitializeViService());

        // Note: using a wrapper here (ViRootServiceGetDisplayService) since the GetDisplayService request IDs are different depending on the service - thus the wrapper helps here
        const bool privileged = g_ViRootServiceType != service::vi::RootServiceType::Application;
        BIO_RES_TRY(ViRootServiceGetDisplayService(privileged, g_ViApplicationDisplayService));

        BIO_RES_TRY(g_ViApplicationDisplayService->GetRelayService(g_HOSBinderDriver));

        BIO_RES_TRY(g_ViApplicationDisplayService->GetSystemDisplayService(g_ViSystemDisplayService));

        g_Initialized = true;
        crt0::RegisterAtExit(reinterpret_cast<crt0::AtExitFunction>(&Finalize), nullptr);
        return ResultSuccess;
    }

    void Finalize() {
        if(g_Initialized) {
            mem::Free(g_TransferMemory);
            svc::CloseHandle(g_TransferMemoryHandle);
            g_TransferMemoryHandle = InvalidHandle;

            auto &nvdrv = GetNvDrvServiceSession();
            service::nv::ErrorCode _err;
            nvdrv->Close(g_NvHostCtrlFd, _err);
            nvdrv->Close(g_NvMapFd, _err);
            nvdrv->Close(g_NvHostFd, _err);

            g_Initialized = false;
        }
    }

    Result GetHOSBinderDriver(mem::SharedObject<service::dispdrv::HOSBinderDriver> &out_service) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_service = g_HOSBinderDriver;
        return ResultSuccess;
    }

    Result GetNvDrvService(mem::SharedObject<service::nv::DrvService> &out_service) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_service = GetNvDrvServiceSession();
        return ResultSuccess;
    }

    Result GetApplicationDisplayService(mem::SharedObject<service::vi::ApplicationDisplayService> &out_service) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_service = g_ViApplicationDisplayService;
        return ResultSuccess;
    }

    Result GetNvMapFd(u32 &out_fd) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_fd = g_NvMapFd;
        return ResultSuccess;
    }

    Result GetNvHostFd(u32 &out_fd) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_fd = g_NvHostFd;
        return ResultSuccess;
    }

    Result GetNvHostCtrlFd(u32 &out_fd) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        out_fd = g_NvHostCtrlFd;
        return ResultSuccess;
    }

    Result CreateStrayLayerSurface(const char *display_name, service::vi::LayerFlags stray_layer_flags, u32 buffer_count, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout, mem::SharedObject<Surface> &out_surface) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);
        
        service::vi::DisplayName name = {};
        util::Strncpy(name.name, display_name, sizeof(name.name));

        u64 display_id;
        BIO_RES_TRY(g_ViApplicationDisplayService->OpenDisplay(name, display_id));

        u64 layer_id;
        ParcelPayload native_window_payload;
        u64 native_window_size;
        BIO_RES_TRY(g_ViApplicationDisplayService->CreateStrayLayer(stray_layer_flags, display_id, &native_window_payload, sizeof(native_window_payload), layer_id, native_window_size));

        BIO_RES_TRY(CreateSurfaceImpl(buffer_count, display_id, layer_id, true, color_fmt, pixel_fmt, layout, native_window_payload, out_surface));
        return ResultSuccess;
    }

}