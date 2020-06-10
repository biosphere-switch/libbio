
#pragma once
#include <bio/gpu/gpu_Results.hpp>
#include <bio/gpu/gpu_Binder.hpp>
#include <bio/util/util_List.hpp>

namespace bio::gpu {

    class Surface {

        private:
            Binder binder;
            QueueBufferOutput qbo;
            void *buffer_data;
            u64 single_buffer_size;
            u32 buffer_count;
            util::LinkedList<bool> slot_has_requested; // Is a linked list the best idea here? A static array seems to be a waste of space, and libnx uses bitmasks...
            GraphicBuffer graphic_buffer;
            ColorFormat color_fmt;
            PixelFormat pixel_fmt;
            Layout layout;
            u64 display_id;
            u64 layer_id;
            u32 vsync_event_handle;
            u32 buffer_event_handle;
            bool is_stray_layer;

            Result Connect();
            Result Initialize();

            Result Disconnect();
            Result Finalize();

        public:
            constexpr Surface(i32 binder_handle, u32 buffer_count, u64 display_id, u64 layer_id, bool is_stray_layer, ColorFormat color_fmt, PixelFormat pixel_fmt, Layout layout) : binder(binder_handle), qbo(), buffer_data(nullptr), single_buffer_size(0), slot_has_requested(), graphic_buffer(), buffer_count(buffer_count), color_fmt(color_fmt), pixel_fmt(pixel_fmt), layout(layout), display_id(display_id), layer_id(layer_id), is_stray_layer(is_stray_layer) {}

            ~Surface() {
                this->FinalizeAll();
            }

            Result InitializeAll() {
                BIO_RES_TRY(this->Connect());
                BIO_RES_TRY(this->Initialize());

                return ResultSuccess;
            }

            Result FinalizeAll() {
                BIO_RES_TRY(this->Disconnect());
                BIO_RES_TRY(this->Finalize());

                return ResultSuccess;
            }

            Binder &GetBinder() {
                return this->binder;
            }

            u32 GetBufferSize() {
                return this->single_buffer_size;
            }

            Result WaitForVsync() {
                i32 idx;
                BIO_RES_TRY(svc::WaitSynchronization(idx, &this->vsync_event_handle, 1, svc::IndefiniteWait));
                
                return ResultSuccess;
            }

            Result WaitForBuffer() {
                i32 idx;
                BIO_RES_TRY(svc::WaitSynchronization(idx, &this->buffer_event_handle, 1, svc::IndefiniteWait));
                
                return ResultSuccess;
            }

            u32 GetWidth() {
                return this->qbo.width;
            }

            u32 GetHeight() {
                return this->qbo.height;
            }

            ColorFormat GetColorFormat() {
                return this->graphic_buffer.planes[0].color_format;
            }

            Result DequeueBuffer(void *&out_buffer, bool is_async, i32 &out_slot, bool &out_has_fences, MultiFence &out_fences);
            Result WaitFences(MultiFence fences, i32 timeout);
            Result QueueBuffer(i32 slot, MultiFence fences);

    };

}