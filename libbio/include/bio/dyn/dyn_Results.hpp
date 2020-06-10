
#pragma once
#include <bio/base_Results.hpp>

namespace bio::dyn::result {

    BIO_RDEF_MODULE(2);

    BIO_RDEF_DEFINE_RES(InvalidInput, 1);
    BIO_RDEF_DEFINE_RES(MissingDtEntry, 2);
    BIO_RDEF_DEFINE_RES(DuplicatedDtEntry, 3);
    BIO_RDEF_DEFINE_RES(InvalidSymEnt, 4);
    BIO_RDEF_DEFINE_RES(InvalidModuleState, 5);
    BIO_RDEF_DEFINE_RES(InvalidRelocationEntry, 6);
    BIO_RDEF_DEFINE_RES(InvalidRelocationTableSize, 7);
    BIO_RDEF_DEFINE_RES(RelaUnsupportedSymbol, 8);
    BIO_RDEF_DEFINE_RES(UnrecognizedRelocationType, 9);
    BIO_RDEF_DEFINE_RES(InvalidRelocationTableType, 10);
    BIO_RDEF_DEFINE_RES(NeedsSymTab, 11);
    BIO_RDEF_DEFINE_RES(NeedsStrTab, 12);
    BIO_RDEF_DEFINE_RES(CouldNotResolveSymbol, 13);
    BIO_RDEF_DEFINE_RES(RoNotInitialized, 14);

}