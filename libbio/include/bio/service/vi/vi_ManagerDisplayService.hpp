
#pragma once
#include <bio/service/vi/vi_Types.hpp>
#include <bio/service/service_Types.hpp>

namespace bio::service::vi {

    class ManagerDisplayService : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result CreateManagedLayer(LayerFlags flags, u64 aruid, u64 display_id, u64 &out_layer_id) {
                return this->session.SendRequestCommand<2010>(ipc::client::In(flags), ipc::client::In<u32>(0), ipc::client::In(display_id), ipc::client::In(aruid), ipc::client::Out(out_layer_id));
            }

            inline Result DestroyManagedLayer(u64 layer_id) {
                return this->session.SendRequestCommand<2011>(ipc::client::In(layer_id));
            }

            inline Result CreateStrayLayer(LayerFlags layer_flags, u64 display_id, void *out_native_window, u64 native_window_size, u64 &out_layer_id, u64 &out_native_window_size) {
                return this->session.SendRequestCommand<2012>(ipc::client::In<LayerFlags>(layer_flags), ipc::client::In<u32>(0), ipc::client::In<u64>(display_id), ipc::client::Buffer(out_native_window, native_window_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias), ipc::client::Out<u64>(out_layer_id), ipc::client::Out<u64>(out_native_window_size));
            }

    };

}