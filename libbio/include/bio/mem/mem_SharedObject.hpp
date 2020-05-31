
#pragma once
#include <bio/mem/mem_Memory.hpp>
#include <bio/util/util_Templates.hpp>

#include <bio/svc/svc_Impl.hpp>

namespace bio::mem {

    // Note: original implementation for this type: https://github.com/SRombauts/shared_ptr

    template<typename T>
    class SharedObject {

        private:
            class RefCount {

                private:
                    i64 *pn;

                public:
                    RefCount() : pn(nullptr) {}
                    RefCount(const RefCount &count) : pn(count.pn) {}

                    void Swap(RefCount &count) {
                        util::Swap(this->pn, count.pn);
                    }

                    i64 UseCount() {
                        if(this->pn != nullptr) {
                            return *pn;
                        }
                        return 0;
                    }

                    template<class U>
                    void Acquire(U *p) {
                        if(p != nullptr) {
                            if(this->pn == nullptr) {
                                mem::AllocateSingle(this->pn); // Assert?
                                *this->pn = 1;
                            }
                            else {
                                ++(*this->pn);
                            }
                        }
                    }

                    template<class U>
                    void Release(U *p) {
                        if(this->pn != nullptr) {
                            --(*this->pn);
                            if(*this->pn == 0) {
                                mem::Delete(p);
                                mem::Free(this->pn);
                                this->pn = nullptr;
                            }
                        }
                    }

            };

        private:
            T *px;
            RefCount pn;

            void Acquire(T *p) {
                this->pn.Acquire(p);
                this->px = p;
            }

        public:
            constexpr SharedObject() : px(nullptr), pn() {}

            SharedObject(T *p) : pn() {
                this->Acquire(p);
            }

            template<class U>
            SharedObject(const SharedObject<U> &ptr, T *p) : pn(ptr.pn) {
                this->Acquire(p);
            }

            template<class U>
            SharedObject(const SharedObject<U> &ptr) : pn(ptr.pn) {
                // Assert ptr.px == nullptr || ptr.pn.UseCount() != 0
                this->Acquire(reinterpret_cast<T*>(ptr.px));
            }

            SharedObject(const SharedObject &ptr) : pn(ptr.pn) {
                // Assert ptr.px == nullptr || ptr.pn.UseCount() != 0
                this->Acquire(ptr.px);
            }

            SharedObject &operator=(SharedObject ptr) {
                this->Swap(ptr);
                return *this;
            }

            ~SharedObject() {
                this->Release();
            }

            void Reset() {
                this->Release();
            }

            void Dispose() {
                this->Reset();
            }

            void Reset(T *p) {
                // Assert p == nullptr || p != this->px
                this->Release();
                this->Acquire(p);
            }

            void Release() {
                this->pn.Release(this->px);
                this->px = nullptr;
            }

            void Swap(SharedObject &ptr) {
                util::Swap(this->px, ptr.px);
                this->pn.Swap(ptr.pn);
            }

            i64 GetUseCount() {
                return this->pn.UseCount();
            }

            bool IsValid() {
                return this->GetUseCount() > 0;
            }

            bool IsUnique() {
                return this->GetUseCount() == 1;
            }

            T &operator*() const {
                return *this->Get();
            }

            T *operator->() const {
                return this->Get();
            }

            T *Get() const {
                return this->px;
            }

    };

    template<class T, class U>
    inline bool operator==(const SharedObject<T> &l, const SharedObject<U> &r) {
        return (l.Get() == r.Get());
    }

    template<class T, class U>
    inline bool operator!=(const SharedObject<T> &l, const SharedObject<U> &r) {
        return (l.Get() != r.Get());
    }

    template<typename T, typename ...Args>
    inline Result NewShared(SharedObject<T> &out_obj, Args &&...args) {
        T *obj;
        BIO_RES_TRY(New<T>(obj, args...));
        
        out_obj = util::Move(SharedObject<T>(obj));
        return ResultSuccess;
    }

}