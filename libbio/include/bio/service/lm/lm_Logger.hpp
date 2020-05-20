
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::lm {

    enum class LogDestination : u32 {
        TMA = BIO_BITMASK(0),
        UART = BIO_BITMASK(1),
        UARTSleeping = BIO_BITMASK(2),
        All = 0xFFFF,
    };

    class Logger : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result Log(void *buf, u64 buf_size) {
                return this->session.SendRequestCommand<0>(ipc::client::Buffer(buf, buf_size, ipc::client::BufferAttribute::In | ipc::client::BufferAttribute::AutoSelect));
            }

            inline Result SetDestination(LogDestination destination) {
                return this->session.SendRequestCommand<1>(ipc::client::In<u32>(static_cast<u32>(destination)));
            }

    };

}