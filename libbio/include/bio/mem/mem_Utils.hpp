
#pragma once
#include <bio/mem/mem_Results.hpp>

namespace bio::mem {

	template<typename T>
	inline u8 *At(T *ptr, u64 index) {
		return reinterpret_cast<u8*>(ptr) + index;
	}

	inline void Set(void *ptr, u64 index, u8 value) {
		*At(ptr, index) = value;
	}

	inline u8 Get(const void *ptr, u64 index) {
		return *At(const_cast<void*>(ptr), index);
	}

	inline void Fill(void *ptr, u8 value, u64 count) {
		if(ptr != nullptr) {
			for(u64 i = 0; i < count; i++) {
				Set(ptr, i, value);
			}
		}
	}

	inline void Zero(void *ptr, u64 size) {
		Fill(ptr, 0, size);
	}
	
	template<typename T>
	inline void ZeroCount(T *ptr, u64 count) {
		Zero(ptr, sizeof(T) * count);
	}

	template<typename T>
	inline void ZeroSingle(T *ptr) {
		ZeroCount(ptr, 1);
	}

	template<typename T, u64 N>
	inline void ZeroArray(T (&arr)[N]) {
		ZeroCount(arr, N);
	}

	template<typename T>
	inline T Zeroed() {
		T t;
		ZeroSingle(&t);
		return t;
	}

	inline void Copy(void *dest_ptr, const void *src_ptr, u64 size) {
		if(dest_ptr != nullptr) {
			if(src_ptr != nullptr) {
				for(u64 i = 0; i < size; i++) {
					Set(dest_ptr, i, Get(src_ptr, i));
				}
			}
		}
	}

	template<typename T>
	inline void Copy(T *dest, const T *src) {
		Copy(dest, src, sizeof(T));
	}

	inline bool IsAddressAligned(void *addr, u64 align) {
		const auto addr64 = reinterpret_cast<u64>(addr);
		const auto inv_mask = align - 1;
		return (addr64 & inv_mask) == 0;
	}

	template<typename T>
	inline constexpr T AlignUp(T value, u64 align) {
		const auto inv_mask = align - 1;
		return static_cast<T>((value + inv_mask) & ~inv_mask);
	}

}