
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/svc/svc_Impl.hpp>
#include <bio/mem/mem_Memory.hpp>

using namespace bio;

/*
namespace bio::crt0 {

    __attribute__((section(".module_name")))
    volatile auto g_ModuleName = CRT0_MAKE_MODULE_NAME("my-own-module-name");

}
*/

void Main() {

    svc::OutputDebugString("BRUH", __builtin_strlen("BRUH"));

}