
#pragma once
#include <bio/input/input_Types.hpp>
#include <bio/input/input_Results.hpp>

namespace bio::input {

    class Player {

        private:
            service::hid::ControllerId controller;
            ControllerData *data;
            u64 prev_button_state;

            inline constexpr ControllerStateEntry *GetLatestStateEntry() {
                return &this->data->main.entries[this->data->main.latest_index];
            }

            inline constexpr u64 GetButtonState() {
                return this->GetLatestStateEntry()->button_state;
            }

        public:
            constexpr Player(service::hid::ControllerId controller, ControllerData *player_data) : controller(controller), data(player_data), prev_button_state(0) {}

            inline constexpr service::hid::ControllerId GetControllerId() {
                return this->controller;
            }

            inline constexpr bool AreJoyConsJoined() {
                return !static_cast<bool>(this->data->is_joycon_half);
            }

            inline constexpr Key GetButtonStateHeld() {
                auto button_state = this->GetButtonState();
                this->prev_button_state = button_state;
                return static_cast<Key>(button_state);
            }

            inline constexpr bool ButtonStateHeldMatches(Key key) {
                return static_cast<bool>(this->GetButtonStateHeld() & key);
            }

            inline constexpr Key GetButtonStateDown() {
                auto button_state = this->GetButtonState();
                auto down_state = ~this->prev_button_state & button_state;
                this->prev_button_state = button_state;
                return static_cast<Key>(down_state);
            }

            inline constexpr bool ButtonStateDownMatches(Key key) {
                return static_cast<bool>(this->GetButtonStateDown() & key);
            }

            inline constexpr Key GetButtonStateUp() {
                auto button_state = this->GetButtonState();
                auto up_state = this->prev_button_state & ~button_state;
                this->prev_button_state = button_state;
                return static_cast<Key>(up_state);
            }

            inline constexpr bool ButtonStateUpMatches(Key key) {
                return static_cast<bool>(this->GetButtonStateUp() & key);
            }

    };

    Result Initialize(u64 aruid, service::hid::NpadStyleTag supported_tags, util::SizedArray<service::hid::ControllerId, 9> &controllers);
    void Finalize();

    Result GetAppletResource(mem::SharedObject<service::hid::AppletResource> &out_applet_res);
    Result GetSharedMemoryData(SharedMemoryData *&out_data);

    bool IsControllerConnected(service::hid::ControllerId controller);
    Result GetPlayer(service::hid::ControllerId controller, mem::SharedObject<Player> &out_player);
    Result GetMainPlayer(mem::SharedObject<Player> &out_player);

}