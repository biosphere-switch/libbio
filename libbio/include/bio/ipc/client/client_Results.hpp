
#pragma once
#include <bio/base_Results.hpp>

namespace bio::ipc::client::result {

    RES_DEF_MODULE(300);

    RES_DEF_DEFINE(InvalidInput, 1);
    RES_DEF_DEFINE(InvalidRequestCommandResponse, 2);

}