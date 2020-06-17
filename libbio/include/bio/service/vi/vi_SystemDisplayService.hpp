
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/vi/vi_Types.hpp>

namespace bio::service::vi {

    class SystemDisplayService : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result GetZOrderCountMin(u64 display_id, i64 &out_z) {
                return this->session.SendRequestCommand<1200>(ipc::client::In(display_id), ipc::client::Out(out_z));
            }

            inline Result GetZOrderCountMax(u64 display_id, i64 &out_z) {
                return this->session.SendRequestCommand<1202>(ipc::client::In(display_id), ipc::client::Out(out_z));
            }

            inline Result SetLayerPosition(u64 layer_id, f32 x, f32 y) {
                return this->session.SendRequestCommand<2201>(ipc::client::In<f32>(x), ipc::client::In<f32>(y), ipc::client::In<u64>(layer_id));
            }

            inline Result SetLayerSize(u64 layer_id, u64 width, u64 height) {
                return this->session.SendRequestCommand<2203>(ipc::client::In<u64>(layer_id), ipc::client::In<u64>(width), ipc::client::In<u64>(height));
            }

            inline Result SetLayerZ(u64 layer_id, i64 z) {
                return this->session.SendRequestCommand<2205>(ipc::client::In<u64>(layer_id), ipc::client::In<i64>(z));
            }

            inline Result CreateStrayLayer(LayerFlags layer_flags, u64 display_id, void *out_native_window, u64 native_window_size, u64 &out_layer_id, u64 &out_native_window_size) {
                return this->session.SendRequestCommand<2312>(ipc::client::In<LayerFlags>(layer_flags), ipc::client::In<u32>(0), ipc::client::In<u64>(display_id), ipc::client::Buffer(out_native_window, native_window_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias), ipc::client::Out<u64>(out_layer_id), ipc::client::Out<u64>(out_native_window_size));
            }

    };

}