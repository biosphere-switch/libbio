
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

namespace bio::diag {

    AssertMode g_DefaultAssertMode = AssertMode::DiagLog;

}

void Main() {
    BIO_DIAG_LOG("Main()");
}