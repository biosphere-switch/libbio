
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/os/os_Mutex.hpp>

using namespace bio;

os::Mutex g_DebugLogLock;

void call(void*) {
    u32 c = 0;
    while(true) {
        os::ScopedMutexLock lk(g_DebugLogLock);
        BIO_DIAG_LOGF("Count: %d", c);

        c++;
        svc::SleepThread(1'000'000'000);
    }
}

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

void Main() {
    DEBUG_LOG_FMT("Main()");

    mem::SharedObject<os::ThreadObject> thread1;
    CRT0_RES_ASSERT(os::ThreadObject::Create(&call, nullptr, 0x1000, "Thread1", thread1));
    mem::SharedObject<os::ThreadObject> thread2;
    CRT0_RES_ASSERT(os::ThreadObject::Create(&call, nullptr, 0x1000, "Thread2", thread2));
    mem::SharedObject<os::ThreadObject> thread3;
    CRT0_RES_ASSERT(os::ThreadObject::Create(&call, nullptr, 0x1000, "Thread3", thread3));

    CRT0_RES_ASSERT(thread1->Start());
    CRT0_RES_ASSERT(thread2->Start());
    CRT0_RES_ASSERT(thread3->Start());
    call(nullptr);
}