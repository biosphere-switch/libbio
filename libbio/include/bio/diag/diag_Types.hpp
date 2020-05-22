
#pragma once
#include <bio/base.hpp>

namespace bio::diag {

    struct SourceInfo {
        u32 line_number;
        const char *file_name;
        const u32 file_name_len;
        const char *function_name;
        const u32 function_name_len;
    };

}