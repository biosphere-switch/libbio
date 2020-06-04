
#pragma once
#include <bio/ipc/client/client_SessionCommandArguments.hpp>
#include <bio/os/os_Mutex.hpp>

namespace bio::service {

    template<typename S>
    inline Result CreateService(mem::SharedObject<S> &out_srv_obj) {
        static_assert(ipc::client::IsValidSessionType<S>);
        return ipc::client::CreateSessionObject(out_srv_obj);
    }

    template<typename S>
    class ServiceGuard {

        private:
            i32 ref_count;
            os::Mutex lock;
            mem::SharedObject<S> obj_ref;

        public:
            constexpr ServiceGuard() : ref_count(0), lock(), obj_ref() {}

            inline Result Initialize() {
                os::ScopedMutexLock lk(this->lock);
                if(this->ref_count == 0) {
                    BIO_RES_TRY(CreateService(this->obj_ref));
                    this->ref_count++;
                }
                return ResultSuccess;
            }

            inline Result InitializeWith(mem::SharedObject<S> &obj) {
                this->obj_ref = obj;
                return this->Initialize();
            }

            inline bool IsInitialized() {
                os::ScopedMutexLock lk(this->lock);
                return this->ref_count > 0;
            }

            inline void Finalize() {
                os::ScopedMutexLock lk(this->lock);
                if(this->ref_count > 0) {
                    this->ref_count--;
                    if(this->ref_count == 0) {
                        this->obj_ref.Dispose();
                    }
                }
            }

            inline mem::SharedObject<S> &Get() {
                return this->obj_ref;
            }

            inline S *operator->() {
                return this->obj_ref.operator->();
            }

    };

    template<typename S>
    class ScopedSessionGuard {

        private:
            ServiceGuard<S> &srv_guard;
            Result init_rc;

        public:
            ScopedSessionGuard(ServiceGuard<S> &guard) : srv_guard(guard) {
                this->init_rc = this->srv_guard.Initialize();
            }
            ~ScopedSessionGuard() {
                this->srv_guard.Finalize();
            }

            inline Result GetResult() {
                return this->init_rc;
            }

            inline operator Result() {
                return this->GetResult();
            }

    };

}

#define BIO_SERVICE_DEFINE_GLOBAL_SESSION(ns, type) \
namespace bio::service::ns { \
    ServiceGuard<type> type ## Session; \
}

#define BIO_SERVICE_DECLARE_GLOBAL_SESSION(type) extern ::bio::service::ServiceGuard<type> type ## Session;
