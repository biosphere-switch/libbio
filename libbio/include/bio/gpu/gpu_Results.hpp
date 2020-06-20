
#pragma once
#include <bio/base_Results.hpp>
#include <bio/service/nv/nv_DrvService.hpp>
#include <bio/gpu/gpu_Types.hpp>

namespace bio::gpu::result {

    BIO_RDEF_MODULE(8);

    BIO_RDEF_DEFINE_RES(NotInitialized, 1);

    BIO_RDEF_DEFINE_RES(BinderErrorCodeUnknown, 2);
    BIO_RDEF_DEFINE_RES(BinderErrorCodePermissionDenied, 3);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeNameNotFound, 4);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeWouldBlock, 5);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeNoMemory, 6);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeAlreadyExists, 7);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeNoInit, 8);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeBadValue, 9);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeDeadObject, 10);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeInvalidOperation, 11);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeNotEnoughData, 12);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeUnknownTransaction, 13);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeBadIndex, 14);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeTimeOut, 15);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeFdsNotAllowed, 16);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeFailedTransaction, 17);
    BIO_RDEF_DEFINE_RES(BinderErrorCodeBadType, 18);

    BIO_RDEF_DEFINE_RES(FdsNotSupported, 19); // Fds are not supported in this implementation (result used mostly when we are given fds).
    BIO_RDEF_DEFINE_RES(ParcelReadSizeMismatch, 20);
    BIO_RDEF_DEFINE_RES(InvalidIoctlFd, 21);
    BIO_RDEF_DEFINE_RES(NotEnoughReadSpace, 22);
    BIO_RDEF_DEFINE_RES(NotEnoughWriteSpace, 23);
    BIO_RDEF_DEFINE_RES(InvalidServiceType, 24);

    inline constexpr Result ConvertBinderErrorCode(BinderErrorCode err) {
        #define _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(name) \
        case BinderErrorCode::name: \
            return ResultBinderErrorCode ## name;

        switch(err) {
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(PermissionDenied)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(NameNotFound)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(WouldBlock)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(NoMemory)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(AlreadyExists)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(NoInit)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(BadValue)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(DeadObject)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(InvalidOperation)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(NotEnoughData)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(UnknownTransaction)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(BadIndex)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(TimeOut)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(FdsNotAllowed)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(FailedTransaction)
            _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT(BadType)
            default:
                return ResultBinderErrorCodeUnknown;
        }
        
        #undef _BIO_GPU_BINDER_ERROR_CODE_CASE_CONVERT
    }

}