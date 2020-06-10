
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/util/util_List.hpp>
#include <bio/fs/fs_Api.hpp>
#include <bio/os/os_Version.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("dev-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::ProcessExit;

}

class AccService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "acc:u0";
        }

    public:
        inline Result GetUserCount(u32 &out_count) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_count));
        }

};

class BioDevService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "bio-dev";
        }

    public:
        inline Result Sample0(u32 &out_v) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_v));
        }

};

void DevMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<BioDevService> biodev;
    BIO_DIAG_RES_ASSERT(service::CreateService(biodev));

    u32 val;
    BIO_DIAG_RES_ASSERT(biodev->Sample0(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

void AccMitmMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<AccService> acc;
    BIO_DIAG_RES_ASSERT(service::CreateService(acc));

    u32 val;
    BIO_DIAG_RES_ASSERT(acc->GetUserCount(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

#define FS_LOG(fmt, ...) ({ \
    mem::SharedObject<fs::File> f; \
    BIO_DIAG_RES_ASSERT(fs::OpenFile("sd:/fs_test_log.log", service::fsp::FileOpenMode::Write | service::fsp::FileOpenMode::Append, f)); \
    char buf[0x400] = {}; \
    const auto buf_len = static_cast<u32>(util::SNPrintf(buf, sizeof(buf), fmt, ##__VA_ARGS__)); \
    BIO_DIAG_RES_ASSERT(f->Write(buf, buf_len)); \
})

void FsMain() {
    BIO_DIAG_LOG("Main()");

    BIO_DIAG_RES_ASSERT(service::fsp::FileSystemServiceSession.Initialize());
    BIO_DIAG_RES_ASSERT(fs::MountSdCard("sd"));
    
    fs::CreateFile("sd:/file", service::fsp::FileAttribute::None, 0);
    fs::DeleteFile("sd:/file");

    fs::CreateDirectory("sd:/dir");
    fs::CreateDirectory("sd:/dir/dir2");
    fs::CreateDirectory("sd:/dir/../newdir");

    service::fsp::DirectoryEntryType type;
    fs::GetEntryType("sd:/newdir", type);

    BIO_DIAG_LOGF("Type: %s", (type == service::fsp::DirectoryEntryType::Directory) ? "Directory" : "File");

    BIO_DIAG_LOG("Done");
}

#include <bio/gpu/gpu_Impl.hpp>

#define MY_ASSERT(...) BIO_DIAG_DETAILED_RES_ASSERT(diag::AssertMode::Fatal, __VA_ARGS__)

class SurfaceBuffer {

    public:
        static constexpr u32 XMask = ~0x7B4;
        static constexpr u32 YMask = 0x7B4;

    private:
        u32 *buf;
        u32 buf_size;
        u32 width;
        u32 height;
        gpu::ColorFormat color_fmt;

        template<u32 Mask>
        inline constexpr i32 SwizzleMask(u32 value) {
            u32 out = 0;
            for(i32 shift = 0; shift < 32; shift++) {
                u32 bit = 1u << shift;
                if(Mask & bit) {
                    if(value & 1) {
                        out |= bit;
                    }
                    value >>= 1;
                }
            }
            return out;
        }

        inline constexpr i32 SwizzleX(u32 value) {
            return this->SwizzleMask<XMask>(value);
        }

        inline constexpr i32 SwizzleY(u32 value) {
            return this->SwizzleMask<YMask>(value);
        }

        inline constexpr u32 Clamp(i32 coord, u32 max) {
            if(coord < 0) {
                return 0;
            }
            if(coord > max) {
                return max;
            }
            return static_cast<u32>(coord);
        }

        inline constexpr bool IsValid() {
            if(this->buf != nullptr) {
                if(this->buf_size > 0) {
                    if(this->width > 0) {
                        if(this->height > 0) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

    public:
        constexpr SurfaceBuffer() : buf(nullptr), buf_size(0), width(0), height(0), color_fmt() {}
        constexpr SurfaceBuffer(u32 *buf, u32 buf_size, u32 width, u32 height, gpu::ColorFormat color_fmt) : buf(buf), buf_size(buf_size), width(width), height(height), color_fmt(color_fmt) {}

        void Clear(u32 color) {
            if(this->IsValid()) {
                for(u64 i = 0; i < this->buf_size / sizeof(color); i++) {
                    *(buf + i) = color;
                }
                // mem::Fill<u32>(this->buf, color, this->buf_size / sizeof(color));
            }
        }

        void BlitWithColor(i32 x, i32 y, u32 w, u32 h, u32 color) {
            if(this->IsValid()) {
                auto x0 = Clamp(x, this->width);
                auto x1 = Clamp(x + w, this->width);
                auto y0 = Clamp(y, this->height);
                auto y1 = Clamp(y + w, this->height);
                
                const auto bpp = gpu::CalculateBytesPerPixel(this->color_fmt);
                const auto aligned_w = gpu::AlignWidth(bpp, this->width);
                const auto y_increment = SwizzleX(aligned_w);
                
                auto x0_offset = SwizzleX(x0) + y_increment * (y0 / gpu::BlockHeight);
                auto y0_offset = SwizzleY(y0);
                for(auto y = y0; y < y1; y++) {
                    auto buf_line = this->buf + y0_offset;
                    auto x_offset = x0_offset;
                    for(auto x = x0; x < x1; x++) {
                        buf_line[x_offset] = color;
                        x_offset = (x_offset - XMask) & XMask;
                    }
                    y0_offset = (y0_offset - YMask) & YMask;
                    if(y0_offset == 0) {
                       x0_offset += y_increment;
                    }
                }
            }
        }

};

void Main() {
    MY_ASSERT(gpu::Initialize(service::nv::DrvServiceType::Applet, 0x800000));
    {
        const auto color_fmt = gpu::ColorFormat::A8B8G8R8;
        const auto pixel_fmt = gpu::PixelFormat::RGBA_8888;
        const auto layout = gpu::Layout::BlockLinear;
        const auto buf_count = 2;
        mem::SharedObject<gpu::Surface> surface;
        MY_ASSERT(gpu::CreateStrayLayerSurface("Default", service::vi::LayerFlags::Default, buf_count, color_fmt, pixel_fmt, layout, surface));
        
        u32 x = 0;
        u32 y = 0;
        u32 w = 120;
        u32 h = 120;
        u32 incr = 1;
        bool x_plus = true;
        bool y_plus = true;
        constexpr u32 royal_blue = 0x41'69'E1'FF;
        constexpr u32 black = 0x00'00'00'FF;
        constexpr u32 white = 0xFF'FF'FF'FF;
        constexpr u32 red = 0xFF'00'00'FF;

        void *buf;
        i32 slot;
        bool has_fence;
        gpu::MultiFence fence;
        
        MY_ASSERT(surface->DequeueBuffer(buf, slot, has_fence, fence));
        SurfaceBuffer s_buf(reinterpret_cast<u32*>(buf), surface->GetBufferSize(), surface->GetWidth(), surface->GetHeight(), surface->GetColorFormat());
        s_buf.Clear(red);
        s_buf.BlitWithColor(x, y, w, h, royal_blue);
        MY_ASSERT(surface->QueueBuffer(slot));

        svc::SleepThread(5'000'000'000ul);
    }
    // gpu::Finalize();
}