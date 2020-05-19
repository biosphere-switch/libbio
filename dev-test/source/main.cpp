
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/service/service_Services.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("custom-module");

void Main() {

    BIO_DIAG_LOG("Hello from libbio!");
    BIO_SERVICE_DO_WITH(sm, _sm_rc, {
        CRT0_RES_ASSERT(_sm_rc);
        BIO_SERVICE_DO_WITH(set::sys, _setsys_rc, {
            CRT0_RES_ASSERT(_setsys_rc);
            service::set::sys::FirmwareVersion fw;
            CRT0_RES_ASSERT(service::set::sys::SysServiceSession->GetFirmwareVersion(fw));
            BIO_DIAG_LOGF("Console firmware version: %s", fw.display_title);
        });
    });

}