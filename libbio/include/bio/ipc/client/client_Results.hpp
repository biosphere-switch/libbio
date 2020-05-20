
#pragma once
#include <bio/base_Results.hpp>

namespace bio::ipc::client::result {

    BIO_RDEF_MODULE(300);

    BIO_RDEF_DEFINE_RES(InvalidInput, 1);
    BIO_RDEF_DEFINE_RES(InvalidRequestCommandResponse, 2);

}