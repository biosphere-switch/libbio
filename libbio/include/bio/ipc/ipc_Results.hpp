
#pragma once
#include <bio/base_Results.hpp>

namespace bio::ipc::result {

    BIO_RDEF_MODULE(3);

    BIO_RDEF_DEFINE_RES(InvalidInput, 1);

}

namespace bio::ipc::cmif::result {

    BIO_RDEF_LEGACY_MODULE(10);

    BIO_RDEF_LEGACY_DEFINE_RES(InvalidHeaderSize, 202);
    BIO_RDEF_LEGACY_DEFINE_RES(InvalidInputHeader, 211);
    BIO_RDEF_LEGACY_DEFINE_RES(InvalidOutputHeader, 212);
    BIO_RDEF_LEGACY_DEFINE_RES(InvalidRequestId, 221);
    BIO_RDEF_LEGACY_DEFINE_RES(OutOfDomainEntries, 301);

}

namespace bio::ipc::hipc::result {

    BIO_RDEF_LEGACY_MODULE(11);

    BIO_RDEF_LEGACY_DEFINE_RES(OutOfServerSessionMemory, 102);

}