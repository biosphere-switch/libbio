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

        Result LaunchMitmQueryThread() {
            os::ScopedMutexLock lk(g_MitmQueryLock);
            if(!g_MitmQueryThreadLaunched) {
                BIO_RES_TRY(os::Thread::Create(&MitmQueryThread, &g_MitmQueryManager, nullptr, 0x4000, 27, -2, "MitmQueryThread", g_MitmQueryThread));
                BIO_RES_TRY(g_MitmQueryThread->Start());
                g_MitmQueryThreadLaunched = true;
            }
            return ResultSuccess;
        }

    }

    Result ServerManager::RegisterMitmQuerySession(u32 mitm_query_handle, ShouldMitmFunction fn) {
        BIO_RES_TRY(g_MitmQueryManager.RegisterSession<MitmQueryServer>(mitm_query_handle, fn));
        BIO_RES_TRY(LaunchMitmQueryThread());
        return ResultSuccess;
    }

}