#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/ipc/server/server_ServerManager.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("custom-service-server-test");

namespace bio::crt0 {

    u8 stacked_heap[0x20000];

    Result InitializeHeap(void *heap_address, u64 heap_size, void *&out_heap_address, u64 &out_size) {
        mem::ZeroArray(stacked_heap);
        out_heap_address = stacked_heap;
        out_size = sizeof(stacked_heap);
        return ResultSuccess;
    };

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::Fatal;

}

#include <bio/ipc/server/server_ServerManager.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

namespace server {

    class BioDevService : public ipc::server::Service {

        public:
            static inline constexpr const char *GetName() {
                return "bio-dev";
            }

            static inline constexpr i32 GetMaxSessions() {
                return 0x10;
            }

        public:
            Result Sample0(ipc::CommandContext &ctx) {
                BIO_DIAG_LOG("Sample0 -> returning value...");

                return this->RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out<u32>(420));
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(0, Sample0),
            };

    };

}

void Main() {
    BIO_DIAG_LOG("Main()");

    ipc::server::ServerManager man;

    BIO_DIAG_RES_ASSERT(man.RegisterServiceServer<server::BioDevService>());

    BIO_DIAG_LOG("Looping process...");
    man.LoopProcess();
}
