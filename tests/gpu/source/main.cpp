#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/gpu/gpu_Impl.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("gpu-test");

    constexpr u64 HeapSize = 128_MB;

    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_heap_size) {
        if(hbl_heap_address != nullptr) {
            out_heap_address = hbl_heap_address;
            out_heap_size = hbl_heap_size;
        }
        else {
            void *heap_addr;
            BIO_RES_TRY(svc::SetHeapSize(heap_addr, HeapSize));

            out_heap_address = heap_addr;
            out_heap_size = HeapSize;
        }
        return ResultSuccess;
    }

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

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
                mem::Fill<u32>(this->buf, color, this->buf_size / sizeof(color));
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

#include <bio/input/input_Impl.hpp>

void Main() {
    const auto color_fmt = gpu::ColorFormat::R8G8B8A8;
    const auto pixel_fmt = gpu::PixelFormat::RGBA_8888;
    const auto layout = gpu::Layout::BlockLinear;
    const u32 buf_count = 2;
    mem::SharedObject<gpu::Surface> surface;
    BIO_DIAG_RES_ASSERT(gpu::CreateStrayLayerSurface("Default", service::vi::LayerFlags::Default, 1280, 720, buf_count, color_fmt, pixel_fmt, layout, surface));
    
    u32 x = 50;
    u32 y = 50;
    u32 w = 100;
    u32 h = 100;
    u32 x_incr = 1;
    u32 y_incr = 1;
    u32 x_mult = 4;
    u32 y_mult = 4;
    const auto width = surface->GetWidth();
    const auto height = surface->GetHeight();

    constexpr u32 royal_blue = 0x41'69'E1'FF;
    constexpr u32 white = 0xFF'FF'FF'FF;

    void *buf;
    i32 slot;
    bool has_fences;
    gpu::MultiFence fences;
    while(true) {
        BIO_DIAG_RES_ASSERT(surface->DequeueBuffer(buf, true, slot, has_fences, fences));

        SurfaceBuffer s_buf(reinterpret_cast<u32*>(buf), surface->GetBufferSize(), width, height, surface->GetColorFormat());
        s_buf.Clear(white);
        s_buf.BlitWithColor(x, y, w, h, royal_blue);
        
        x += x_incr * x_mult;
        y += y_incr * y_mult;

        if(x <= 0) {
            if(x_incr < 0) {
                x_incr = -x_incr;
            }
            x += x_incr * x_mult;
            x_mult++;
        }
        else if((x + w) >= width) {
            if(x_incr > 0) {
                x_incr = -x_incr;
            }
            x += x_incr * x_mult;
            x_mult++;
        }
        if(y <= 0) {
            if(y_incr < 0) {
                y_incr = -y_incr;
            }
            y += y_incr * y_mult;
            y_mult++;
        }
        else if((y + h) >= height) {
            if(y_incr > 0) {
                y_incr = -y_incr;
            }
            y += y_incr * y_mult;
            y_mult++;
        }

        BIO_DIAG_RES_ASSERT(surface->WaitForVsync());
        BIO_DIAG_RES_ASSERT(surface->QueueBuffer(slot, fences));
    }
}