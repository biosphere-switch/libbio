#include <bio/crt0/crt0_ModuleName.hpp>

namespace bio::crt0 {

    // Default module name - applications can override this (don't forget the section attribute)
    // Volatile -> the compiler won't erase it for being unused

    __attribute__((weak))
    __attribute__((section(".module_name")))
    volatile auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("libbio");

}
