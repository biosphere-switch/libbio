
#pragma once
#include <bio/service/service_Types.hpp>

namespace bio::service::fatal {

    enum class Policy : u32 {
        ErrorReportAndErrorScreen,
        ErrorReport,
        ErrorScreen,
    };

}