
#pragma once
#include <bio/base.hpp>

__attribute__((visibility("hidden")))
inline void* operator new(unsigned long, void* __p) {
	return __p; 
}

__attribute__((visibility("hidden")))
inline void* operator new[](unsigned long, void* __p) {
	return __p;
}

__attribute__((visibility("hidden")))
inline void operator delete(void*, void*) {}

__attribute__((visibility("hidden")))
inline void operator delete[](void*, void*) {}

namespace bio::mem {

	void Initialize(void *address, u64 size);

	void *AllocateAligned(u64 alignment, u64 size);
	void Free(void *ptr);

	inline void Set(void *ptr, u64 index, u8 value) {
		reinterpret_cast<u8*>(ptr)[index] = value;
	}

	inline u8 Get(const void *ptr, u64 index) {
		return reinterpret_cast<const u8*>(ptr)[index];
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

	constexpr u64 PageAlignment = 0x1000;

	template<typename T = void>
	inline T *Allocate(u64 size) {
		return reinterpret_cast<T*>(AllocateAligned(PageAlignment, size));
	}

	template<typename T>
	inline T *AllocateCount(u64 count) {
		return Allocate<T>(sizeof(T) * count);
	}

	template<typename T>
	inline T *AllocateSingle() {
		return AllocateCount<T>(1);
	}

	template<typename T, typename ...Args>
	inline T *New(Args &&...args) {
		auto obj = AllocateSingle<T>();
		if(obj != nullptr) {
			new (obj) T(args...);
		}
		return obj;
	}

	template<typename T>
	inline void Delete(T *ptr) {
		if(ptr != nullptr) {
			Free(ptr);
			ptr->~T();
		}
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