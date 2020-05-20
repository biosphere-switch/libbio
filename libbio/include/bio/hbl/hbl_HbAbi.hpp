
#pragma once
#include <bio/base.hpp>

namespace bio::hbl {

    struct ABIConfigEntry {
        u32 key;
        u32 flags;
        u64 value[2];
    };

    enum class ABIConfigKey {
        EOL = 0,
        MainThreadHandle = 1,
        NextLoadPath = 2,
        OverrideHeap = 3,
        OverrideService = 4,
        Argv = 5,
        SyscallAvailableHint = 6,
        AppletType = 7,
        AppletWorkaround = 8,
        Reserved9 = 9,
        ProcessHandle = 10,
        LastLoadResult = 11,
        RandomSeed = 14,
        UserIdStorage = 15,
        HosVersion = 16,
    };

    enum class ABIEntryFlags : u32 {
        Mandatory = BIO_BITMASK(0),
    };

    enum class ABIAppletFlags : u32 {
        ApplicationOverride = BIO_BITMASK(0),
    };

}