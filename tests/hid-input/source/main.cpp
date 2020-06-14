
#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/input/input_Impl.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("hid-input-test");

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

void Main() {
    BIO_DIAG_LOG("Main()");

    const u64 aruid = 0;
    const auto supported_tags = service::hid::NpadStyleTag::Handheld | service::hid::NpadStyleTag::JoyconLeft | service::hid::NpadStyleTag::JoyconRight | service::hid::NpadStyleTag::JoyconPair | service::hid::NpadStyleTag::ProController | service::hid::NpadStyleTag::SystemExt | service::hid::NpadStyleTag::System;
    util::SizedArray<service::hid::ControllerId, 9> controllers = { service::hid::ControllerId::Player1, service::hid::ControllerId::Player2, service::hid::ControllerId::Player3, service::hid::ControllerId::Player4, service::hid::ControllerId::Player5, service::hid::ControllerId::Player6, service::hid::ControllerId::Player7, service::hid::ControllerId::Player8, service::hid::ControllerId::Handheld };
    BIO_DIAG_RES_ASSERT(input::Initialize(aruid, supported_tags, controllers));

    while(true) {
        mem::SharedObject<input::Player> player;
        if(input::IsControllerConnected(service::hid::ControllerId::Handheld)) {
            BIO_DIAG_RES_ASSERT(input::GetPlayer(service::hid::ControllerId::Handheld, player));
        }
        else if(input::IsControllerConnected(service::hid::ControllerId::Player1)) {
            BIO_DIAG_RES_ASSERT(input::GetPlayer(service::hid::ControllerId::Player1, player));
        }
        else {
            BIO_DIAG_LOG("No controller connected");
            continue;
        }

        if(player->ButtonStateDownMatches(input::Key::A)) {
            BIO_DIAG_LOG("Key A was pressed");
            break;
        }

        svc::SleepThread(100'000'000ul);
    }
}