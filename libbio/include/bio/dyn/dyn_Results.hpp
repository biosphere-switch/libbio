
#pragma once
#include <bio/base_Results.hpp>

namespace bio::dyn::result {

    RES_DEF_MODULE(200);

    RES_DEF_DEFINE(InvalidInput, 1);
    RES_DEF_DEFINE(MissingDtEntry, 2);
    RES_DEF_DEFINE(DuplicatedDtEntry, 3);
    RES_DEF_DEFINE(InvalidSymEnt, 4);
    RES_DEF_DEFINE(InvalidModuleState, 5);
    RES_DEF_DEFINE(InvalidRelocEnt, 6);
    RES_DEF_DEFINE(InvalidRelocTableSize, 7);
    RES_DEF_DEFINE(RelaUnsupportedSymbol, 8);
    RES_DEF_DEFINE(UnrecognizedRelocType, 9);
    RES_DEF_DEFINE(InvalidRelocTableType, 10);
    RES_DEF_DEFINE(NeedsSymTab, 11);
    RES_DEF_DEFINE(NeedsStrTab, 12);
    RES_DEF_DEFINE(CouldNotResolveSymbol, 13);

}