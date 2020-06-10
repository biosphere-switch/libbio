
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/vi/vi_Types.hpp>

namespace bio::service::vi {

    class SystemDisplayService : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result SetLayerPosition(u64 layer_id, f32 x, f32 y) {
                return this->session.SendRequestCommand<2201>(ipc::client::In<u64>(layer_id), ipc::client::In<f32>(x), ipc::client::In<f32>(y));
            }

            inline Result SetLayerSize(u64 layer_id, u64 width, u64 height) {
                return this->session.SendRequestCommand<2203>(ipc::client::In<u64>(layer_id), ipc::client::In<u64>(width), ipc::client::In<u64>(height));
            }

            inline Result SetLayerZ(u64 layer_id, i64 z) {
                return this->session.SendRequestCommand<2205>(ipc::client::In<u64>(layer_id), ipc::client::In<i64>(z));
            }

    };

}