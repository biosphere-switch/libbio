
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/os/os_Tls.hpp>
#include <bio/os/os_Mutex.hpp>

using namespace bio;

namespace bio::diag {

    AssertMode g_DefaultAssertMode = AssertMode::DiagLog;

}

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

void Main() {
    BIO_DIAG_LOG("Main()");
    BIO_DIAG_DETAILED_ASSERT(diag::AssertMode::SvcBreak, diag::g_DefaultAssertMode == diag::AssertMode::Default);
}