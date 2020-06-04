
#pragma once
#include <bio/mem/mem_Utils.hpp>

// These are needed to be able to call the constructor via placement new

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

	Result AllocateAligned(u64 alignment, u64 size, void *&out_address);
	void Free(void *address);

	bool IsAllocated(void *address);
	bool IsFree(void *address);

	AllocationInfo GetAllocationInfo(void *address);

	constexpr u64 NoAlignment = 0;

	constexpr u64 PageAlignment = 0x1000;

	template<u64 Alignment = NoAlignment, typename T = void>
	inline Result Allocate(u64 size, T *&out) {
		return AllocateAligned(Alignment, size, reinterpret_cast<void*&>(out));
	}

	template<u64 Alignment = NoAlignment, typename T>
	inline Result AllocateCount(u64 count, T *&out) {
		return Allocate<Alignment, T>(sizeof(T) * count, out);
	}

	template<u64 Alignment = NoAlignment, typename T>
	inline Result AllocateSingle(T *&out) {
		return AllocateCount<Alignment, T>(1, out);
	}

	template<u64 Alignment = NoAlignment, typename T, typename ...Args>
	inline Result New(T *&out, Args &&...args) {
		BIO_RES_TRY(AllocateSingle<Alignment>(out));
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

}