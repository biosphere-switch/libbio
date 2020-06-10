
#pragma once
#include <bio/base.hpp>

namespace bio::mem {

	void FlushDataCache(void *address, u64 size);
    void CleanDataCache(void *address, u64 size);
    void InvalidateInstructionCache(void *address, u64 size);
    void ZeroDataCache(void *address, u64 size);

}