
#pragma once
#include <bio/base_Results.hpp>

namespace bio::input::result {

    BIO_RDEF_MODULE(10);

    BIO_RDEF_DEFINE_RES(NotInitialized, 1);
    BIO_RDEF_DEFINE_RES(InvalidController, 2);

}