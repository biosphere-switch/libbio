
#pragma once
#include <bio/base_Results.hpp>

namespace bio::ipc::server::result {

    BIO_RDEF_MODULE(500);

    BIO_RDEF_DEFINE_RES(Unsupported, 1);
    BIO_RDEF_DEFINE_RES(InvalidRequestCommandResponse, 2);

}