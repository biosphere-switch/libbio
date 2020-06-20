
#pragma once
#include <bio/service/nv/nv_Types.hpp>
#include <bio/base_Results.hpp>

namespace bio::service::nv::result {

    BIO_RDEF_MODULE(7);

    BIO_RDEF_DEFINE_RES(ErrorCodeUnknown, 1);
    BIO_RDEF_DEFINE_RES(ErrorCodeNotImplemented, 2);
    BIO_RDEF_DEFINE_RES(ErrorCodeNotSupported, 3);
    BIO_RDEF_DEFINE_RES(ErrorCodeNotInitialized, 4);
    BIO_RDEF_DEFINE_RES(ErrorCodeInvalidParameter, 5);
    BIO_RDEF_DEFINE_RES(ErrorCodeTimeOut, 6);
    BIO_RDEF_DEFINE_RES(ErrorCodeInsufficientMemory, 7);
    BIO_RDEF_DEFINE_RES(ErrorCodeReadOnlyAttribute, 8);
    BIO_RDEF_DEFINE_RES(ErrorCodeInvalidState, 9);
    BIO_RDEF_DEFINE_RES(ErrorCodeInvalidAddress, 10);
    BIO_RDEF_DEFINE_RES(ErrorCodeInvalidSize, 11);
    BIO_RDEF_DEFINE_RES(ErrorCodeInvalidValue, 12);
    BIO_RDEF_DEFINE_RES(ErrorCodeAlreadyAllocated, 13);
    BIO_RDEF_DEFINE_RES(ErrorCodeBusy, 14);
    BIO_RDEF_DEFINE_RES(ErrorCodeResourceError, 15);
    BIO_RDEF_DEFINE_RES(ErrorCodeCountMismatch, 16);
    BIO_RDEF_DEFINE_RES(ErrorCodeSharedMemoryTooSmall, 17);
    BIO_RDEF_DEFINE_RES(ErrorCodeFileOperationFailed, 18);
    BIO_RDEF_DEFINE_RES(ErrorCodeIoctlFailed, 19);

    inline constexpr Result ConvertErrorCode(ErrorCode err) {
        #define _BIO_NV_ERROR_CODE_CASE_CONVERT(name) \
        case ErrorCode::name: \
            return ResultErrorCode ## name;

        switch(err) {
            _BIO_NV_ERROR_CODE_CASE_CONVERT(NotImplemented)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(NotSupported)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(NotInitialized)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InvalidParameter)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(TimeOut)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InsufficientMemory)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(ReadOnlyAttribute)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InvalidState)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InvalidAddress)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InvalidSize)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(InvalidValue)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(AlreadyAllocated)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(Busy)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(ResourceError)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(CountMismatch)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(SharedMemoryTooSmall)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(FileOperationFailed)
            _BIO_NV_ERROR_CODE_CASE_CONVERT(IoctlFailed)
            default:
                return ResultErrorCodeUnknown;
        }
        
        #undef _BIO_NV_ERROR_CODE_CASE_CONVERT
    }

}