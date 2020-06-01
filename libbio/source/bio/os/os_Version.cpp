#include <bio/os/os_Version.hpp>
#include <bio/os/os_Mutex.hpp>

namespace bio::os {

    namespace {

        Version g_Version;
        os::Mutex g_VersionLock;

    }

    Version GetSystemVersion() {
        os::ScopedMutexLock lk(g_VersionLock);
        return g_Version;
    }

    void SetSystemVersion(Version ver) {
        os::ScopedMutexLock lk(g_VersionLock);
        g_Version = ver;
    }

}