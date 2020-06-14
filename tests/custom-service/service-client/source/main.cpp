
#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("custom-service-client-test");

    constexpr u64 HeapSize = 128_MB;

    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_heap_size) {
        if(hbl_heap_address != nullptr) {
            out_heap_address = hbl_heap_address;
            out_heap_size = hbl_heap_size;
        }
        else {
            void *heap_addr;
            BIO_RES_TRY(svc::SetHeapSize(heap_addr, HeapSize));

            out_heap_address = heap_addr;
            out_heap_size = HeapSize;
        }
        return ResultSuccess;
    }

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

class BioDevService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "bio-dev";
        }

    public:
        inline Result Sample0(u32 &out_v) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_v));
        }

};

void Main() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<BioDevService> biodev;
    BIO_DIAG_RES_ASSERT(service::CreateService(biodev));

    u32 val;
    BIO_DIAG_RES_ASSERT(biodev->Sample0(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    BIO_DIAG_LOG("Done");
}