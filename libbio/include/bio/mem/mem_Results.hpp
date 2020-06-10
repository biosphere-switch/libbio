
#pragma once
#include <bio/base_Results.hpp>

namespace bio::mem::result {

    BIO_RDEF_MODULE(1);

    BIO_RDEF_DEFINE_RES(InvalidSize, 1);
    BIO_RDEF_DEFINE_RES(OutOfMemory, 2);
    BIO_RDEF_DEFINE_RES(OutOfAllocationTableSpace, 3);

}