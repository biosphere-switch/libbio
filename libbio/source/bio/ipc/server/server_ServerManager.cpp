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

        Result EnsureMitmQueryThreadLaunched() {
            if(!g_MitmQueryThreadLaunched) {
                // TODO: get minimum thread priority allowed in this process (svc::GetInfo and info type 1 / PriorityMask)
                BIO_RES_TRY(os::Thread::Create(&MitmQueryThread, &g_MitmQueryManager, nullptr, 0x4000, 20, os::Thread::DefaultCpuId, "MitmQueryThread", g_MitmQueryThread));
                BIO_RES_TRY(g_MitmQueryThread->Start());
                g_MitmQueryThreadLaunched = true;
            }
            return ResultSuccess;
        }

    }

    Result ServerManager::RegisterMitmQuerySession(u32 mitm_query_handle, ShouldMitmFunction fn) {
        os::ScopedMutexLock lk(g_MitmQueryLock);
        BIO_RES_TRY(g_MitmQueryManager.RegisterSession<MitmQueryServer>(mitm_query_handle, fn));
        BIO_RES_TRY(EnsureMitmQueryThreadLaunched());
        return ResultSuccess;
    }

}