#include <bio/os/os_Version.hpp>
#include <bio/os/os_Mutex.hpp>

namespace bio::os {

    Version g_SystemVersion;

    namespace {

        os::Mutex g_SystemVersionLock;

    }

    Version GetSystemVersion() {
        os::ScopedMutexLock lk(g_SystemVersionLock);
        return g_SystemVersion;
    }

}