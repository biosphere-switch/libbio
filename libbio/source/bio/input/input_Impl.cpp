#include <bio/input/input_Impl.hpp>
#include <bio/mem/mem_VirtualMemory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/service/service_Services.hpp>

namespace bio::input {

    namespace {

        bool g_Initialized = false;
        mem::SharedObject<service::hid::AppletResource> g_HidAppletResource;
        u32 g_SharedMemoryHandle = InvalidHandle;
        u64 g_AppletResourceUserId = 0;
        SharedMemoryData *g_SharedMemoryData = nullptr;

        inline constexpr Result GetIndexForController(service::hid::ControllerId controller, u32 &out_idx) {
            switch(controller) {
                case service::hid::ControllerId::Player1:
                case service::hid::ControllerId::Player2:
                case service::hid::ControllerId::Player3:
                case service::hid::ControllerId::Player4:
                case service::hid::ControllerId::Player5:
                case service::hid::ControllerId::Player6:
                case service::hid::ControllerId::Player7:
                case service::hid::ControllerId::Player8: {
                    out_idx = static_cast<u32>(controller);
                    return ResultSuccess;
                }
                case service::hid::ControllerId::Handheld: {
                    out_idx = 8;
                    return ResultSuccess;
                }
                default:
                    return result::ResultInvalidController;
            }
        }

        Result SetAllControllersModeDual(u64 aruid) {
            #define _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(player) BIO_RES_TRY((service::hid::HidServiceSession->SetNpadJoyAssignmentModeDual(aruid, service::hid::ControllerId::player)))

            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player1);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player2);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player3);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player4);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player5);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player6);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player7);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Player8);
            _BIO_INPUT_SET_CONTROLLER_MODE_DUAL(Handheld);

            #undef _BIO_INPUT_SET_CONTROLLER_MODE_DUAL

            return ResultSuccess;
        }

    }

    Result Initialize(u64 aruid, service::hid::NpadStyleTag supported_tags, util::SizedArray<service::hid::ControllerId, 9> &controllers) {
        BIO_RET_UNLESS(!g_Initialized, ResultSuccess);

        BIO_RES_TRY(service::hid::HidServiceSession.Initialize());

        BIO_RES_TRY(service::hid::HidServiceSession->CreateAppletResource(aruid, g_HidAppletResource));

        BIO_RES_TRY(g_HidAppletResource->GetSharedMemoryHandle(g_SharedMemoryHandle));

        BIO_RES_TRY(mem::AllocateVirtual(g_SharedMemoryData));

        BIO_RES_TRY(svc::MapSharedMemory(g_SharedMemoryHandle, g_SharedMemoryData, sizeof(SharedMemoryData), svc::MemoryPermission::Read));

        BIO_RES_TRY(service::hid::HidServiceSession->ActivateNpad(aruid));

        BIO_RES_TRY(service::hid::HidServiceSession->SetSupportedNpadStyleSet(aruid, supported_tags));

        BIO_RES_TRY(service::hid::HidServiceSession->SetSupportedNpadIdType(aruid, controllers.Get(), controllers.GetSize() * sizeof(service::hid::ControllerId)));

        BIO_RES_TRY(SetAllControllersModeDual(aruid));

        g_AppletResourceUserId = aruid;
        crt0::RegisterAtExit(reinterpret_cast<crt0::AtExitFunction>(&Finalize));
        g_Initialized = true;
        return ResultSuccess;
    }

    void Finalize() {
        if(g_Initialized) {
            SetAllControllersModeDual(g_AppletResourceUserId);
            service::hid::HidServiceSession->DeactivateNpad(g_AppletResourceUserId);
            
            svc::UnmapSharedMemory(g_SharedMemoryHandle, g_SharedMemoryData, sizeof(SharedMemoryData));
            svc::CloseHandle(g_SharedMemoryHandle);
            g_SharedMemoryHandle = InvalidHandle;

            g_Initialized = false;
        }
    }

    Result GetAppletResource(mem::SharedObject<service::hid::AppletResource> &out_applet_res) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);

        out_applet_res = g_HidAppletResource;
        return ResultSuccess;
    }

    Result GetSharedMemoryData(SharedMemoryData *&out_data) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);

        out_data = g_SharedMemoryData;
        return ResultSuccess;
    }

    bool IsControllerConnected(service::hid::ControllerId controller) {
        BIO_RET_UNLESS(g_Initialized, false);

        u32 controller_idx;
        BIO_RET_UNLESS(GetIndexForController(controller, controller_idx).IsSuccess(), false);

        auto controller_state = &g_SharedMemoryData->controllers[controller_idx];
        auto last_entry = &controller_state->main.entries[controller_state->main.latest_index];

        return static_cast<bool>(last_entry->connection_state & ConnectionState::Connected);
    }

    Result GetPlayer(service::hid::ControllerId controller, mem::SharedObject<Player> &out_player) {
        BIO_RET_UNLESS(g_Initialized, result::ResultNotInitialized);

        u32 controller_idx;
        BIO_RES_TRY(GetIndexForController(controller, controller_idx));

        auto controller_data_ref = &g_SharedMemoryData->controllers[controller_idx];
        
        mem::SharedObject<Player> player;
        BIO_RES_TRY(mem::NewShared(player, controller, controller_data_ref));
        out_player = util::Move(player);
        return ResultSuccess;
    }

}