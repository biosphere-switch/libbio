#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/ipc/server/server_ServerManager.hpp>
#include <bio/ipc/server/server_CommandArguments.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("acc-u0-mitm-server-test");

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

    class AccMitmService : public ipc::server::MitmService {

        public:
            static inline constexpr const char *GetName() {
                return "acc:u0";
            }

            static inline constexpr bool ShouldMitm(const service::sm::MitmProcessInfo &info) {
                return true;
            }

        public:
            Result GetUserCount(ipc::CommandContext &ctx) {
                BIO_DIAG_LOG("GetUserCount -> returning stubbed value...");

                return this->RequestCommandEnd(ctx, ResultSuccess, ipc::server::Out<u32>(69));
            }

        public:
            BIO_IPC_SERVER_DECLARE_COMMAND_HANDLERS {
                BIO_IPC_SERVER_COMMAND_HANDLER(0, GetUserCount),
            };

    };

}

void Main() {
    BIO_DIAG_LOG("Main()");

    ipc::server::ServerManager man;

    BIO_DIAG_RES_ASSERT(man.RegisterMitmServiceServer<server::AccMitmService>());

    BIO_DIAG_LOG("Looping process...");
    man.LoopProcess();
}
