
#pragma once
#include <bio/base.hpp>

namespace bio::util {

    namespace impl {

        template<typename T>
        inline constexpr T *GetRawBufferByOffset(void *buf, u64 offset) {
            auto raw_buf = reinterpret_cast<u8*>(buf);
            auto raw_at_offset = raw_buf + offset;
            return reinterpret_cast<T*>(raw_at_offset);
        }

        template<typename T>
        inline constexpr T GetFromBufferByOffset(void *buf, u64 offset) {
            auto raw_t = GetRawBufferByOffset<T>(buf, offset);
            return *raw_t;
        }

        template<typename T>
        inline constexpr void SetAtBufferByOffset(void *buf, u64 offset, T t) {
            auto raw_t = GetRawBufferByOffset<T>(buf, offset);
            *raw_t = t;
        }

    }

    class OffsetCalculator {

        private:
            void *buf;
            u64 tmp_offset;

            template<typename T>
            inline constexpr void ApplyTypeAlignmentImpl() {
                this->tmp_offset += (alignof(T) - 1);
                this->tmp_offset -= (this->tmp_offset % alignof(T));
            }

            template<typename T>
            inline constexpr void ApplyTypeSizeImpl() {
                this->tmp_offset += sizeof(T);
            }

        public:
            constexpr OffsetCalculator() : buf(nullptr), tmp_offset(0) {}
            constexpr OffsetCalculator(void *buf) : buf(buf), tmp_offset(0) {}
            constexpr OffsetCalculator(void *buf, u64 offset) : buf(buf), tmp_offset(offset) {}
            constexpr OffsetCalculator(u64 offset) : buf(nullptr), tmp_offset(offset) {}

            inline constexpr void Reset() {
                this->tmp_offset = 0;
            }

            inline constexpr u64 GetCurrentOffset() {
                return this->tmp_offset;
            }

            template<typename T>
            inline constexpr u64 GetNextOffset() {
                this->ApplyTypeAlignmentImpl<T>();
                auto offset = this->tmp_offset;
                this->ApplyTypeSizeImpl<T>();
                return offset;
            }

            template<typename T>
            inline constexpr T GetByOffset(u64 offset) {
                return impl::GetFromBufferByOffset<T>(this->buf, offset);
            }

            template<typename T>
            inline constexpr void SetByOffset(u64 offset, T t) {
                return impl::SetAtBufferByOffset<T>(this->buf, offset, t);
            }

            template<typename T>
            inline constexpr T GetNext() {
                auto offset = this->GetNextOffset<T>();
                return this->GetByOffset<T>(offset);
            }

            template<typename T>
            inline constexpr void SetNext(T t) {
                auto offset = this->GetNextOffset<T>();
                this->SetByOffset(offset, t);
            }

            template<typename T>
            inline constexpr void IncrementOffset() {
                this->GetNextOffset<T>();
            }

    };

}