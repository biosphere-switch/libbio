
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

            /*
            virtual Result PostInitialize() override {
                return this->Initialize();
            }
            */

        public:
            inline Result LoadNro(void *nro_address, u64 nro_size, void *bss_address, u64 bss_size, u64& out_nro_addr) {
                return this->session.SendSyncRequest<0>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>((u64)nro_address), ipc::client::In<u64>(nro_size), ipc::client::In<u64>((u64)bss_address), ipc::client::In<u64>(bss_size), ipc::client::Out<u64>(out_nro_addr));
            }

            inline Result UnloadNro(void *nro_address) {
                return this->session.SendSyncRequest<1>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>((u64)nro_address));
            }

            inline Result LoadNrr(void *nrr_address, u64 nrr_size) {
                return this->session.SendSyncRequest<2>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>((u64)nrr_address), ipc::client::In<u64>(nrr_size));
            }

            inline Result UnloadNrr(void *nrr_address) {
                return this->session.SendSyncRequest<3>(ipc::client::InProcessId(), ipc::client::In<u64>(0), ipc::client::In<u64>((u64)nrr_address));
            }

            inline Result Initialize() {
                return this->session.SendSyncRequest<4>(ipc::client::InProcessId(), ipc::client::InHandle<ipc::client::HandleMode::Copy>(svc::CurrentProcessPseudoHandle));
            }

    };

    // BIO_SERVICE_DECLARE_GLOBAL_SESSION(RoService);

}