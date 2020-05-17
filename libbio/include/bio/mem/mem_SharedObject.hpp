
#pragma once
#include <bio/mem/mem_Memory.hpp>
#include <bio/util/util_Misc.hpp>

namespace bio::mem {

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
                        DEBUG_LOG_PTR("Acquire", p);
                        if(p != nullptr) {
                            if(this->pn == nullptr) {
                                this->pn = mem::AllocateSingle<i64>();
                                *this->pn = 1;
                            }
                            else {
                                ++(*this->pn);
                            }
                        }
                    }

                    template<class U>
                    void Release(U *p) {
                        DEBUG_LOG_PTR("Release", p);
                        if(this->pn != nullptr) {
                            --(*this->pn);
                            if(*this->pn == 0) {
                                mem::Delete(p);
                                mem::Free(this->pn);
                            }
                            this->pn = nullptr;
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
            SharedObject() : px(nullptr), pn() {}

            explicit SharedObject(T *p) : pn() {
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

            SharedObject &operator=(SharedObject ptr) {
                // Swap
                return *this;
            }

            ~SharedObject() {
                this->Release();
            }

            void Release() {
                this->pn.Release(this->px);
                this->px = nullptr;
            }

            void Release(T *p) {
                // Assert p == nullptr || p != this->px
                this->Release();
                this->Acquire(p);
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
                return *this->px;
            }

            T *operator->() const {
                return this->Get();
            }

            T *Get() const {
                return this->px;
            }

    };

    template<typename T, typename ...Args>
    inline SharedObject<T> NewShared(Args &&...args) {
        auto obj = New<T>(args...);
        return SharedObject<T>(obj);
    }

}