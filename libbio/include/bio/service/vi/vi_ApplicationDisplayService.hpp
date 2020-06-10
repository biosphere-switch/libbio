
#pragma once
#include <bio/service/vi/vi_SystemDisplayService.hpp>
#include <bio/service/dispdrv/dispdrv_HOSBinderDriver.hpp>

namespace bio::service::vi {

    class ApplicationDisplayService : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetRelayService(mem::SharedObject<dispdrv::HOSBinderDriver> &out_service) {
                return this->session.SendRequestCommand<100>(ipc::client::OutSessionObject<0, dispdrv::HOSBinderDriver>(out_service));
            }

            inline Result GetSystemDisplayService(mem::SharedObject<SystemDisplayService> &out_service) {
                return this->session.SendRequestCommand<101>(ipc::client::OutSessionObject<0, SystemDisplayService>(out_service));
            }

            inline Result OpenDisplay(DisplayName name, u64 &out_display_id) {
                return this->session.SendRequestCommand<1010>(ipc::client::In<DisplayName>(name), ipc::client::Out<u64>(out_display_id));
            }

            inline Result CloseDisplay(u64 display_id) {
                return this->session.SendRequestCommand<1020>(ipc::client::In<u64>(display_id));
            }

            inline Result CreateStrayLayer(LayerFlags layer_flags, u64 display_id, void *out_native_window, u64 native_window_size, u64 &out_layer_id, u64 &out_native_window_size) {
                return this->session.SendRequestCommand<2030>(ipc::client::In<LayerFlags>(layer_flags), ipc::client::In<u32>(0), ipc::client::In<u64>(display_id), ipc::client::Buffer(out_native_window, native_window_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias), ipc::client::Out<u64>(out_layer_id), ipc::client::Out<u64>(out_native_window_size));
            }

            inline Result DestroyStrayLayer(u64 layer_id) {
                return this->session.SendRequestCommand<2031>(ipc::client::In<u64>(layer_id));
            }

            inline Result GetDisplayVsyncEvent(u64 display_id, u32 &out_event_handle) {
                return this->session.SendRequestCommand<5202>(ipc::client::In<u64>(display_id), ipc::client::OutHandle<ipc::HandleMode::Copy, 0>(out_event_handle));
            }

    };

}