#include <bio/gpu/gpu_Surface.hpp>
#include <bio/gpu/gpu_Ioctl.hpp>
#include <bio/gpu/gpu_Impl.hpp>

namespace bio::gpu {

    Result Surface::Connect() {
        BIO_RES_TRY(this->binder.Connect(ConnectionApi::Cpu, false, this->qbo));

        // Also get events here.
        mem::SharedObject<service::vi::ApplicationDisplayService> application_display_srv;
        BIO_RES_TRY(GetApplicationDisplayService(application_display_srv));

        BIO_RES_TRY(application_display_srv->GetDisplayVsyncEvent(this->display_id, this->vsync_event_handle));

        BIO_RES_TRY(this->binder.GetBufferEventHandle(this->buffer_event_handle));

        return ResultSuccess;
    }

    Result Surface::Initialize() {
        // TODO: make some more of these customizable...?
        const auto kind = Kind::Generic_16BX2;
        const auto scan_fmt = DisplayScanFormat::Progressive;
        const u32 pid = 42;
        const auto bpp = CalculateBytesPerPixel(this->color_fmt);
        const auto width = this->qbo.width;
        const auto height = this->qbo.height;
        const auto aligned_width = AlignWidth(bpp, width);
        const auto aligned_width_bytes = aligned_width * bpp;
        const auto aligned_height = AlignHeight(height);
        const auto stride = aligned_width;
        this->single_buffer_size = aligned_width_bytes * aligned_height;
        const auto usage = GraphicsAllocatorUsage::HardwareComposer | GraphicsAllocatorUsage::HardwareRender | GraphicsAllocatorUsage::HardwareTexture;
        const auto buf_size = mem::AlignUp(this->buffer_count * this->single_buffer_size, mem::PageAlignment);

        ioctl::nvmap::Create create = {};
        create.size = buf_size;
        BIO_RES_TRY(Ioctl(create));

        ioctl::nvmap::GetId get_id = {};
        get_id.handle = create.handle;
        BIO_RES_TRY(Ioctl(get_id));

        BIO_RES_TRY(mem::Allocate<mem::PageAlignment>(buf_size, this->buffer_data));

        BIO_RES_TRY(svc::SetMemoryAttribute(this->buffer_data, buf_size, 8, svc::MemoryAttribute::Uncached));

        ioctl::nvmap::Alloc alloc = {};
        alloc.handle = create.handle;
        alloc.heap_mask = 0;
        alloc.flags = ioctl::nvmap::AllocFlags::ReadOnly;
        alloc.align = mem::PageAlignment;
        alloc.kind = Kind::Pitch;
        alloc.address = reinterpret_cast<u64>(this->buffer_data);
        BIO_RES_TRY(Ioctl(alloc));

        this->graphic_buffer = {};
        this->graphic_buffer.header.magic = GraphicBufferHeader::Magic;
        this->graphic_buffer.header.width = width;
        this->graphic_buffer.header.height = height;
        this->graphic_buffer.header.stride = stride;
        this->graphic_buffer.header.pixel_format = this->pixel_fmt;
        this->graphic_buffer.header.usage = usage;
        this->graphic_buffer.header.pid = pid;
        this->graphic_buffer.header.buffer_size = (sizeof(GraphicBuffer) - sizeof(GraphicBufferHeader)) / sizeof(u32);
        this->graphic_buffer.map_id = get_id.id;
        this->graphic_buffer.buffer_magic = GraphicBuffer::Magic;
        this->graphic_buffer.pid = pid;
        this->graphic_buffer.usage = usage;
        this->graphic_buffer.pixel_format = this->pixel_fmt;
        this->graphic_buffer.external_pixel_format = this->pixel_fmt;
        this->graphic_buffer.stride = stride;
        this->graphic_buffer.full_size = this->single_buffer_size;
        this->graphic_buffer.plane_count = 1;
        this->graphic_buffer.planes[0].width = width;
        this->graphic_buffer.planes[0].height = height;
        this->graphic_buffer.planes[0].color_format = this->color_fmt;
        this->graphic_buffer.planes[0].layout = this->layout;
        this->graphic_buffer.planes[0].pitch = aligned_width_bytes;
        this->graphic_buffer.planes[0].map_handle_unused = create.handle;
        this->graphic_buffer.planes[0].kind = kind;
        this->graphic_buffer.planes[0].block_height_log2 = BlockHeightLog2;
        this->graphic_buffer.planes[0].display_scan_format = scan_fmt;
        this->graphic_buffer.planes[0].size = this->single_buffer_size;

        this->slot_has_requested.Clear();
        for(u32 i = 0; i < this->buffer_count; i++) {
            auto buf_copy = this->graphic_buffer;
            buf_copy.planes[0].offset = i * this->single_buffer_size;

            BIO_RES_TRY(this->binder.SetPreallocatedBuffer(i, buf_copy));

            this->slot_has_requested.PushBack(false);
        }

        return ResultSuccess;
    }

    Result Surface::Disconnect() {
        BIO_RES_TRY(this->binder.Disconnect(ConnectionApi::Cpu, DisconnectMode::AllLocal));

        return ResultSuccess;
    }

    Result Surface::Finalize() {
        const auto buf_size = mem::AlignUp(this->buffer_count * this->single_buffer_size, mem::PageAlignment);
        BIO_RES_TRY(svc::SetMemoryAttribute(this->buffer_data, buf_size, 0, svc::MemoryAttribute::None));
        mem::Free(this->buffer_data);
        mem::SharedObject<service::vi::ApplicationDisplayService> application_display_srv;
        BIO_RES_TRY(GetApplicationDisplayService(application_display_srv));
        if(this->is_stray_layer) {
            BIO_RES_TRY(application_display_srv->DestroyStrayLayer(this->layer_id));
        }
        else {
            // ...?
        }
        BIO_RES_TRY(application_display_srv->CloseDisplay(this->display_id));

        return ResultSuccess;
    }

    Result Surface::DequeueBuffer(void *&out_buffer, bool is_async, i32 &out_slot, bool &out_has_fences, MultiFence &out_fences) {
        if(is_async) {
            while(true) {
                BIO_RES_TRY(this->WaitForBuffer());
                auto rc = this->binder.DequeueBuffer(true, this->graphic_buffer.header.width, this->graphic_buffer.header.height, false, this->graphic_buffer.usage, out_slot, out_has_fences, out_fences);
                if(rc.IsSuccess()) {
                    break;
                }
                if(rc == result::ResultBinderErrorCodeWouldBlock) {
                    continue;
                }
                BIO_RES_TRY(rc);
            }
        }
        else {
            BIO_RES_TRY(this->binder.DequeueBuffer(false, this->graphic_buffer.header.width, this->graphic_buffer.header.height, false, this->graphic_buffer.usage, out_slot, out_has_fences, out_fences));
        }

        auto has_requested = this->slot_has_requested.GetAt(out_slot);
        if(!has_requested) {
            // Request if we haven't done so.
            bool out_buf_valid;
            GraphicBuffer buf;
            BIO_RES_TRY(this->binder.RequestBuffer(out_slot, out_buf_valid, buf));

            this->slot_has_requested.SetAt(out_slot, true);
        }

        out_buffer = reinterpret_cast<void*>(reinterpret_cast<u8*>(this->buffer_data) + out_slot * this->single_buffer_size);
        return ResultSuccess;
    }

    Result Surface::WaitFences(MultiFence fences, i32 timeout) {
        for(u32 i = 0; i < fences.fence_count; i++) {
            ioctl::nvhostctrl::WaitAsync wait_async;
            wait_async.fence = fences.fences[i];
            wait_async.timeout = timeout;

            BIO_RES_TRY(Ioctl(wait_async));
        }
        return ResultSuccess;
    }

    Result Surface::QueueBuffer(i32 slot, MultiFence fences) {
        QueueBufferInput qbi = {};
        // The other fields are automatically set to zero.
        qbi.swap_interval = 1;
        qbi.fence = fences;

        // Flush data cache.
        const auto buf_size = mem::AlignUp(this->buffer_count * this->single_buffer_size, mem::PageAlignment);
        mem::FlushDataCache(this->buffer_data, buf_size);

        QueueBufferOutput qbo;
        BIO_RES_TRY(this->binder.QueueBuffer(slot, qbi, qbo));

        return ResultSuccess;
    }

}