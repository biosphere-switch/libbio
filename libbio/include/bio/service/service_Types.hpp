
#pragma once
#include <bio/ipc/client/client_Request.hpp>

/*
namespace service {

    template<typename S>
    inline Result CreateService(SharedPointer<S> &out_srv_obj) {
        static_assert(std::is_base_of_v<ipc::client::Service, S>, "Invalid service object");
        return ipc::client::CreateSessionObject(out_srv_obj);
    }

}

#define _BIO_GLOBAL_SESSION_NAME(type) type ## Session

#define BIO_SERVICE_DEFINE_GLOBAL_SESSION(ns, type) \
namespace bio::service::ns { \
    SharedPointer<type> _BIO_GLOBAL_SESSION_NAME(type) = nullptr; \
}

#define BIO_SERVICE_DECLARE_GLOBAL_SESSION(type) \
    extern SharedPointer<type> _BIO_GLOBAL_SESSION_NAME(type); \
    static inline bool IsInitialized() { \
        return util::IsSharedPointerValid(_BIO_GLOBAL_SESSION_NAME(type)); \
    } \
    static inline Result Initialize() { \
        if(IsInitialized()) { \
            return ResultSuccess; \
        } \
        return ipc::client::CreateSessionObject<type>(_BIO_GLOBAL_SESSION_NAME(type)); \
    } \
    static inline void InitializeWith(SharedPointer<type> session) { \
        _BIO_GLOBAL_SESSION_NAME(type) = session; \
    } \
    static inline void Finalize() { \
        util::DisposeSharedPointer(_BIO_GLOBAL_SESSION_NAME(type)); \
    }
*/