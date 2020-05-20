
#pragma once
#include <bio/base.hpp>
#include <bio/mem/mem_Memory.hpp>

namespace bio::util {

    template<typename T, u64 N>
    struct SizedArray {
        T array[N];
        u64 size;

        SizedArray() {
            this->Clear();
        }

        inline constexpr u64 GetMaxLength() {
            return N;
        }

        inline constexpr u64 GetSize() {
            return this->size;
        }

        inline constexpr bool Push(T v) {
            if(this->size < N) {
                this->array[this->size] = v;
                this->size++;
                return true;
            }
            return false;
        }

        inline constexpr bool Pop() {
            if(this->size > 0) {
                this->size--;
                mem::ZeroSingle(&this->array[this->size]);
                return true;
            }
            return false;
        }

        inline constexpr bool IsEmpty() {
            return this->GetSize() == 0;
        }

        inline constexpr bool Any() {
            return !this->IsEmpty();
        }

        inline constexpr bool HasAt(u32 index) {
            return index < this->GetSize();
        }

        inline constexpr T &GetAt(u32 index) {
            return this->array[index];
        }

        inline constexpr T &Front() {
            return this->array[0];
        }

        inline constexpr T &Back() {
            return this->array[this->size - 1];
        }

        inline constexpr void Clear() {
            this->size = 0;
            mem::ZeroArray(this->array);
        }

    };

}