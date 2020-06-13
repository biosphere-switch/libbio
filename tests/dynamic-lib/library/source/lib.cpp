#include <lib.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/crt0/crt0_ModuleName.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("dynamic-lib-library-test");

namespace lib {

    namespace test {

        u32 Sample() {
            return 69420;
        }

    }

}

void Main() {
    crt0::Exit(0xdabe);
}