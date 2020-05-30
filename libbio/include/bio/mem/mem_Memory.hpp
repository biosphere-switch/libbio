
#pragma once
#include <bio/mem/mem_Results.hpp>

__attribute__((visibility("hidden")))
inline void* operator new(unsigned long, void *ptr) {
	return ptr; 
}

__attribute__((visibility("hidden")))
inline void* operator new[](unsigned long, void *ptr) {
	return ptr;
}

__attribute__((visibility("hidden")))
inline void operator delete(void*, void*) {}

__attribute__((visibility("hidden")))
inline void operator delete[](void*, void*) {}

namespace bio::mem {

	struct AllocationInfo {
		u64 address;
		u64 size;

		constexpr AllocationInfo() : address(0), size(0) {}

		inline constexpr bool IsValid() {
			return this->size > 0;
		}

		inline constexpr u64 GetEndAddress() {
			return this->address + this->size;
		}

		inline constexpr bool IsAddressIn(u64 addr) {
			return (this->address <= addr) && (addr < this->GetEndAddress());
		}
		
		inline constexpr bool IsInRegion(u64 addr, u64 size) {
			const auto addr_end = addr + size;
			return ((addr <= this->address) && (this->GetEndAddress() < addr_end)) || IsAddressIn(addr);
		}

		inline constexpr void Clear() {
			this->address = 0;
			this->size = 0;
		}

	};

	void Initialize(void *address, u64 size);

	Result AllocateAligned(u64 alignment, u64 size, void *&out_addr);
	void Free(void *ptr);

	bool IsAllocated(void *address);
	bool IsFree(void *address);

	AllocationInfo GetAllocationInfo(void *address);

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

	constexpr u64 NoAlignment = 0;

	constexpr u64 PageAlignment = 0x1000;

	template<typename T = void>
	inline Result Allocate(u64 size, T *&out) {
		return AllocateAligned(NoAlignment, size, reinterpret_cast<void*&>(out));
	}
	
	template<typename T = void>
	inline Result PageAllocate(u64 size, T *&out) {
		return AllocateAligned(PageAlignment, size, reinterpret_cast<void*&>(out));
	}

	template<typename T>
	inline Result AllocateCount(u64 count, T *&out) {
		return Allocate<T>(sizeof(T) * count, out);
	}

	template<typename T>
	inline Result AllocateSingle(T *&out) {
		return AllocateCount<T>(1, out);
	}

	template<typename T, typename ...Args>
	inline Result New(T *&out, Args &&...args) {
		BIO_RES_TRY(AllocateSingle(out));
		new (out) T(args...);
		return ResultSuccess;
	}

	template<typename T>
	inline void Delete(T *ptr) {
		if(ptr != nullptr) {
			ptr->~T();
			Free(ptr);
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