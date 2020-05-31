
#pragma once
#include <bio/base_Results.hpp>

namespace bio::fs::result {

    BIO_RDEF_MODULE(600);

    BIO_RDEF_DEFINE_RES(FspNotInitialized, 1);
    BIO_RDEF_DEFINE_RES(InvalidPath, 2);
    BIO_RDEF_DEFINE_RES(DeviceNotFound, 3);

}