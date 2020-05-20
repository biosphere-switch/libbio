
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::ro {

    class RoService final : public ipc::client::Service {
        
        public:
            using Service::Service;

            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "ldr:ro";
            }

            inline Result PostInitialize() {
                return this->Initialize();
            }

        public:
            inline Result LoadNro(void *nro_address, u64 nro_size, void *bss_address, u64 bss_size, u64& out_nro_addr) {
                return this->session.SendRequestCommand<0>(ipc::client::InProcessId(), ipc::client::In<u64>(reinterpret_cast<u64>(nro_address)), ipc::client::In<u64>(nro_size), ipc::client::In<u64>(reinterpret_cast<u64>(bss_address)), ipc::client::In<u64>(bss_size), ipc::client::Out<u64>(out_nro_addr));
            }

            inline Result UnloadNro(void *nro_address) {
                return this->session.SendRequestCommand<1>(ipc::client::InProcessId(), ipc::client::In<u64>(reinterpret_cast<u64>(nro_address)));
            }

            inline Result LoadNrr(void *nrr_address, u64 nrr_size) {
                return this->session.SendRequestCommand<2>(ipc::client::InProcessId(), ipc::client::In<u64>(reinterpret_cast<u64>(nrr_address)), ipc::client::In<u64>(nrr_size));
            }

            inline Result UnloadNrr(void *nrr_address) {
                return this->session.SendRequestCommand<3>(ipc::client::InProcessId(), ipc::client::In<u64>(reinterpret_cast<u64>(nrr_address)));
            }

            inline Result Initialize() {
                return this->session.SendRequestCommand<4>(ipc::client::InProcessId(), ipc::client::InHandle<ipc::client::HandleMode::Copy>(svc::CurrentProcessPseudoHandle));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(RoService);

}