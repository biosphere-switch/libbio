
#pragma once
#include <bio/mem/mem_Results.hpp>

namespace bio::mem {

	template<typename T, typename U>
	inline T *At(U *ptr, u64 index) {
		return reinterpret_cast<T*>(ptr) + index;
	}

	template<typename T>
	inline void Set(void *ptr, u64 index, T value) {
		*At<T>(ptr, index) = value;
	}

	template<typename T>
	inline T Get(const void *ptr, u64 index) {
		return *At<T>(const_cast<void*>(ptr), index);
	}

	template<typename T = u8>
	inline void Fill(void *ptr, T value, u64 count) {
		if(ptr != nullptr) {
			for(u64 i = 0; i < count; i++) {
				Set<T>(ptr, i, value);
			}
		}
	}

	template<typename T = u8>
	inline void Zero(void *ptr, u64 size) {
		Fill<T>(ptr, static_cast<T>(0), size);
	}
	
	template<typename T, typename U = u8>
	inline void ZeroCount(T *ptr, u64 count) {
		Zero<U>(ptr, sizeof(T) * count);
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

	template<typename T = u8>
	inline void Copy(void *dest_ptr, const void *src_ptr, u64 size) {
		if(dest_ptr != nullptr) {
			if(src_ptr != nullptr) {
				for(u64 i = 0; i < size; i++) {
					Set<T>(dest_ptr, i, Get<T>(src_ptr, i));
				}
			}
		}
	}

	template<typename T, typename U = u8>
	inline void Copy(T *dest, const T *src) {
		Copy<U>(dest, src, sizeof(T));
	}

	template<typename T>
	inline constexpr bool IsAligned(T value, u64 align) {
		return (static_cast<u64>(value) & (align - 1)) == 0;
	}

	inline bool IsAddressAligned(void *addr, u64 align) {
		return IsAligned(reinterpret_cast<u64>(addr), align);
	}

	template<typename T>
	inline constexpr T AlignUp(T value, u64 align) {
		const auto inv_mask = align - 1;
		return static_cast<T>((value + inv_mask) & ~inv_mask);
	}

}