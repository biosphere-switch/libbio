#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/ipc/server/server_ServerManager.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("custom-service-server-test");

    constexpr u64 HeapSize = 128_KB;
    u8 g_HeapStack[HeapSize];

    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_heap_size) {
        mem::ZeroArray(g_HeapStack);
        out_heap_address = g_HeapStack;
        out_heap_size = HeapSize;
        return ResultSuccess;
    }

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::Fatal;

}

namespace server {

    class BioDevService : public ipc::server::Service {

        public:
            static inline constexpr const char *GetName() {
                return "bio-dev";
            }

            static inline constexpr i32 GetMaxSessions() {
                return 0x10;
            }

        private:
            u32 inner_value;

        public:
            BioDevService() : inner_value(0) {}

            void SetValue32(ipc::CommandContext &ctx) {
                u32 value;
                RequestCommandBegin(ctx, ipc::server::In(value));
                BIO_DIAG_LOGF("SetValue32 -> storing %d...", value);

                this->inner_value = value;

                RequestCommandEnd(ctx, ResultSuccess);
            }

            void GetValue32(ipc::CommandContext &ctx) {
                BIO_DIAG_LOGF("GetValue32 -> returning %d...", this->inner_value);

                RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out(this->inner_value));
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(0, SetValue32),
                BIO_IPC_SERVER_COMMAND_HANDLER(1, GetValue32),
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
