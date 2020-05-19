
#pragma once
#include <bio/svc/svc_Impl.hpp>
#include <bio/os/os_Thread.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/ipc/client/client_Results.hpp>

namespace bio::ipc::client {

    constexpr u32 InvalidHandle = 0;
    constexpr u32 InvalidObjectId = 0;

    struct SessionBase {

        u32 handle;
        u32 object_id;
        bool owns_handle;
        u16 pointer_buffer_size;

        constexpr SessionBase() : handle(InvalidHandle), object_id(InvalidObjectId), owns_handle(false), pointer_buffer_size(0) {}
        constexpr SessionBase(u32 handle) : handle(handle), object_id(InvalidObjectId), owns_handle(true), pointer_buffer_size(0) {}
        constexpr SessionBase(u32 handle, u32 object_id) : handle(handle), object_id(object_id), owns_handle(false), pointer_buffer_size(0) {}

        inline constexpr u32 GetHandle() {
            return this->handle;
        }

        inline constexpr u32 GetObjectId() {
            return this->object_id;
        }

        inline constexpr u16 GetPointerBufferSize() {
            return this->pointer_buffer_size;
        }

        inline constexpr bool IsValid() {
            return this->handle != InvalidHandle;
        }

        inline constexpr bool IsDomain() {
            return this->object_id != InvalidObjectId;
        }

    };

}