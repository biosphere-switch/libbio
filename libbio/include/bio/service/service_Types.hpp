
#pragma once
#include <bio/ipc/client/client_Request.hpp>

namespace bio::service {

    template<typename S>
    inline Result CreateService(mem::SharedObject<S> &out_srv_obj) {
        static_assert(ipc::client::IsService<S>, "Invalid service object");
        return ipc::client::CreateSessionObject(out_srv_obj);
    }

}

#define _BIO_GLOBAL_SESSION_NAME(type) type ## Session

#define BIO_SERVICE_DEFINE_GLOBAL_SESSION(ns, type) \
namespace bio::service::ns { \
    mem::SharedObject<type> _BIO_GLOBAL_SESSION_NAME(type) = nullptr; \
}

#define BIO_SERVICE_DECLARE_GLOBAL_SESSION(type) \
    extern mem::SharedObject<type> _BIO_GLOBAL_SESSION_NAME(type); \
    static inline bool IsInitialized() { \
        return _BIO_GLOBAL_SESSION_NAME(type).IsValid(); \
    } \
    static inline Result Initialize() { \
        if(IsInitialized()) { \
            return ResultSuccess; \
        } \
        return ipc::client::CreateSessionObject<type>(_BIO_GLOBAL_SESSION_NAME(type)); \
    } \
    static inline void InitializeWith(mem::SharedObject<type> session) { \
        _BIO_GLOBAL_SESSION_NAME(type) = session; \
    } \
    static inline void Finalize() { \
        _BIO_GLOBAL_SESSION_NAME(type).Reset(); \
    }