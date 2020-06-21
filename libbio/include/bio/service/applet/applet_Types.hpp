
#pragma once
#include <bio/base.hpp>

namespace bio::service::applet {

    using AppletResourceUserId = u64;

    struct AppletAttribute {
        u8 unk_flag;
        u8 reserved[0x7F];
    };
    static_assert(sizeof(AppletAttribute) == 0x80);

    enum class AppletId : u32 {
        Application = 0x1,
        OverlayDisp = 0x2,
        Qlaunch = 0x3,
        Starter = 0x4,
        Auth = 0xA,
        Cabinet = 0xB,
        Controller = 0xC,
        DataErase = 0xD,
        Error = 0xE,
        NetConnect = 0xF,
        PlayerSelect = 0x10,
        Swkbd = 0x11,
        MiiEdit = 0x12,
        Web = 0x13,
        Shop = 0x14,
        PhotoViewer = 0x15,
        Set = 0x16,
        OfflineWeb = 0x17,
        LoginShare = 0x18,
        WifiWebAuth = 0x19,
        MyPage = 0x1A,
        // TODO: add non-retail IDs too?
    };

    enum class LibraryAppletMode : u32 {
        AllForeground,
        Background,
        NoUi,
        BackgroundIndirectDisplay,
        AllForegroundInitiallyHidden,
    };

}