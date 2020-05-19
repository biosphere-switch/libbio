
#include <bio/service/service_Services.hpp>

BIO_SERVICE_DEFINE_GLOBAL_SESSION(sm, UserNamedPort);
BIO_SERVICE_DEFINE_GLOBAL_SESSION(ro, RoService);
BIO_SERVICE_DEFINE_GLOBAL_SESSION(set::sys, SysService);
BIO_SERVICE_DEFINE_GLOBAL_SESSION(fsp, FileSystemService);
BIO_SERVICE_DEFINE_GLOBAL_SESSION(lm, LogService);