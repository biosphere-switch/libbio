
#pragma once
#include <bio/service/hid/hid_Service.hpp>

namespace bio::input {

    struct TouchData {
        u64 timestamp;
        u32 pad1;
        u32 index;
        u32 x;
        u32 y;
        u32 diameter_x;
        u32 diameter_y;
        u32 angle;
        u32 pad2;
    };

    struct TouchEntry {
        u64 timestamp;
        u64 count;
        TouchData youches[16];
        u64 pad1;
    };

    struct TouchState {
        u64 timestamp_ticks;
        u64 entry_count;
        u64 latest_index;
        u64 max_index;
        u64 tmestamp;
        TouchEntry entries[17];
    };

    struct JoystickPosition {
        u32 x;
        u32 y;
    };

    enum class ConnectionState : u64 {
        None = 0,
        Connected = BIO_BITMASK(0),
        Wired = BIO_BITMASK(1),
    };

    BIO_ENUM_BIT_OPERATORS(ConnectionState, u64)

    struct ControllerStateEntry {
        u64 timestamp;
        u64 timestamp2;
        u64 button_state;
        JoystickPosition left_pos;
        JoystickPosition right_pos;
        ConnectionState connection_state;
    };

    struct ControllerState {
        u64 timestamp;
        u64 entry_count;
        u64 latest_index;
        u64 max_index;
        ControllerStateEntry entries[17];
    };

    struct ControllerMACAddress {
        u8 address[0x10];
    };

    struct ControllerColor {
        u32 body;
        u32 buttons;
    };

    struct ControllerData {
        u32 status;
        u32 is_joycon_half;
        u32 colors_descriptor_single;
        ControllerColor color_single;
        u32 colors_descriptor_split;
        ControllerColor color_right;
        ControllerColor color_left;
        ControllerState pro_controller;
        ControllerState handleld;
        ControllerState joined;
        ControllerState left;
        ControllerState right;
        ControllerState main_no_analog;
        ControllerState main;
        u8 unk[0x2a78];
        ControllerMACAddress macs[0x2];
        u8 unk2[0xe10];
    };

    struct SharedMemoryData {
        u8 header[0x400];
        TouchState touch_state;
        u8 pad[0x3c0];
        u8 mouse[0x400];
        u8 keyboard[0x400];
        u8 unk[0x400];
        u8 unk2[0x400];
        u8 unk3[0x400];
        u8 unk4[0x400];
        u8 unk5[0x200];
        u8 unk6[0x200];
        u8 unk7[0x200];
        u8 unk8[0x800];
        u8 controller_serials[0x4000];
        ControllerData controllers[10];
        u8 unk9[0x4600];
    };
    static_assert(sizeof(SharedMemoryData) == 0x40000);

}