
#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/service/applet/applet_AllSystemAppletProxiesService.hpp>
#include <bio/os/os_Wait.hpp>
#include <bio/arm/arm_Tick.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("libapplet-launch-test");

    constexpr u64 HeapSize = 16_MB;

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

void Main() {
    BIO_DIAG_LOG("Main()");

    BIO_DIAG_RES_ASSERT(service::applet::AllSystemAppletProxiesServiceSession.Initialize());

    mem::SharedObject<service::applet::LibraryAppletProxy> lib_applet_proxy;
    service::applet::AppletAttribute attr = {};
    BIO_DIAG_RES_ASSERT(service::applet::AllSystemAppletProxiesServiceSession->OpenLibraryAppletProxy(attr, lib_applet_proxy));

    mem::SharedObject<service::applet::LibraryAppletCreator> lib_applet_creator;
    BIO_DIAG_RES_ASSERT(lib_applet_proxy->GetLibraryAppletCreator(lib_applet_creator));

    mem::SharedObject<service::applet::LibraryAppletAccessor> psel_accessor;
    BIO_DIAG_RES_ASSERT(lib_applet_creator->CreateLibraryApplet(service::applet::AppletId::PlayerSelect, service::applet::LibraryAppletMode::AllForeground, psel_accessor));

    struct {
        u32 version;
        u32 size;
        u32 lib_applet_api_version;
        u32 theme_color;
        bool play_startup_sound;
        u8 pad[7];
        u64 system_tick;
    } common_args = { 1, sizeof(common_args), 0x20000, 0, false, {0}, arm::GetSystemTick() };

    {
        mem::SharedObject<service::applet::Storage> st;
        BIO_DIAG_RES_ASSERT(lib_applet_creator->CreateStorage(sizeof(common_args), st));

        {
            mem::SharedObject<service::applet::StorageAccessor> st_access;
            BIO_DIAG_RES_ASSERT(st->Open(st_access));

            BIO_DIAG_RES_ASSERT(st_access->Write(0, &common_args, sizeof(common_args)));
        }
    }

    u8 applet_data[0xA0] = {};
    applet_data[0x96] = 1;

    {
        mem::SharedObject<service::applet::Storage> st;
        BIO_DIAG_RES_ASSERT(lib_applet_creator->CreateStorage(sizeof(applet_data), st));

        {
            mem::SharedObject<service::applet::StorageAccessor> st_access;
            BIO_DIAG_RES_ASSERT(st->Open(st_access));

            BIO_DIAG_RES_ASSERT(st_access->Write(0, applet_data, sizeof(applet_data)));
        }
    }

    u32 event_handle;
    BIO_DIAG_RES_ASSERT(psel_accessor->GetAppletStateChangedEvent(event_handle));

    BIO_DIAG_RES_ASSERT(psel_accessor->Start());

    i32 tmp_idx;
    BIO_DIAG_RES_ASSERT(os::WaitHandles(svc::IndefiniteWait, tmp_idx, event_handle));

    svc::CloseHandle(event_handle);

    BIO_DIAG_LOG("Done");
}