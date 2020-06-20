#include <bio/ipc/server/server_ServerManager.hpp>

namespace bio::ipc::server {

    namespace {

        ServerManager g_MitmQueryManager;
        mem::SharedObject<os::Thread> g_MitmQueryThread;
        os::Mutex g_MitmQueryLock;
        bool g_MitmQueryThreadLaunched = false;

        void MitmQueryThread(void *manager_v) {
            auto manager = reinterpret_cast<ServerManager*>(manager_v);
            manager->LoopProcess();
        }

    }

    ServerManager &GetMitmQueryManager() {
        os::ScopedMutexLock lk(g_MitmQueryLock);
        return g_MitmQueryManager;
    }

    Result EnsureMitmQueryThreadLaunched() {
        os::ScopedMutexLock lk(g_MitmQueryLock);
        if(!g_MitmQueryThreadLaunched) {
            // TODO: get minimum thread priority allowed in this process (svc::GetInfo and info type 1 / PriorityMask)
            BIO_RES_TRY(os::Thread::Create(&MitmQueryThread, &g_MitmQueryManager, nullptr, 0x4000, 20, os::Thread::DefaultCpuId, "MitmQueryThread", g_MitmQueryThread));
            BIO_RES_TRY(g_MitmQueryThread->Start());
            g_MitmQueryThreadLaunched = true;
        }
        return ResultSuccess;
    }

}