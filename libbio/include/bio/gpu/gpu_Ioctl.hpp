
#pragma once
#include <bio/gpu/gpu_Surface.hpp>
#include <bio/service/vi/vi_RootService.hpp>

namespace bio::gpu {

    enum class IoctlMode : u8 {
        In = BIO_BITMASK(0),
        Out = BIO_BITMASK(1),
    };

    BIO_ENUM_BIT_OPERATORS(IoctlMode, u8);

    enum class IoctlFd : u8 {
        NvHost,
        NvMap,
        NvHostCtrl,
    };

    template<typename T>
    concept IsIoctl = requires(T) {
        { T::Id } -> util::SameAs<const service::nv::IoctlId>;
        { T::Mode } -> util::SameAs<const IoctlMode>;
        { T::Fd } -> util::SameAs<const IoctlFd>;
    };

    Result DoIoctl(IoctlFd fd, service::nv::IoctlId ioctl_id, IoctlMode mode, void *data, u64 data_size);

    template<typename T>
    inline Result Ioctl(T &ioctl_type) {
        static_assert(IsIoctl<T>);
        return DoIoctl(T::Fd, T::Id, T::Mode, &ioctl_type, sizeof(ioctl_type));
    }

    // Ioctl implementations

    namespace ioctl {

        namespace nvmap {

            struct Create {
                u32 size;
                u32 handle;

                static constexpr auto Mode = IoctlMode::In | IoctlMode::Out;
                static constexpr auto Id = service::nv::IoctlId::NvMapCreate;
                static constexpr auto Fd = IoctlFd::NvMap;
            };

            struct FromId {
                u32 id;
                u32 handle;

                static constexpr auto Mode = IoctlMode::In | IoctlMode::Out;
                static constexpr auto Id = service::nv::IoctlId::NvMapFromId;
                static constexpr auto Fd = IoctlFd::NvMap;
            };

            enum class AllocFlags : u32 {
                ReadOnly = 0,
                ReadWrite = 1,
            };

            struct Alloc {
                u32 handle;
                u32 heap_mask;
                AllocFlags flags;
                u32 align;
                Kind kind;
                u8 pad[4];
                u64 address;

                static constexpr auto Mode = IoctlMode::In | IoctlMode::Out;
                static constexpr auto Id = service::nv::IoctlId::NvMapAlloc;
                static constexpr auto Fd = IoctlFd::NvMap;
            };

            struct GetId {
                u32 id;
                u32 handle;

                static constexpr auto Mode = IoctlMode::In | IoctlMode::Out;
                static constexpr auto Id = service::nv::IoctlId::NvMapGetId;
                static constexpr auto Fd = IoctlFd::NvMap;
            };

        }

        namespace nvhostctrl {

            struct WaitAsync {
                Fence fence;
                i32 timeout;

                static constexpr auto Mode = IoctlMode::In;
                static constexpr auto Id = service::nv::IoctlId::NvHostCtrlWaitAsync;
                static constexpr auto Fd = IoctlFd::NvHostCtrl;
            };

        }

    }

}