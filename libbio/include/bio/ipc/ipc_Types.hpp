
#pragma once
#include <bio/os/os_Tls.hpp>
#include <bio/ipc/ipc_Results.hpp>
#include <bio/util/util_List.hpp>
#include <bio/util/util_Templates.hpp>
#include <bio/util/util_Offset.hpp>

namespace bio::ipc {

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

    enum class HandleMode {
        Copy,
        Move
    };

    enum class BufferMode {
        Normal,
        NonSecure,
        Invalid,
        NonDevice,
    };

    struct BufferDescriptor {
        u32 size_low;
        u32 address_low;
        u32 mode : 2;
        u32 address_high : 22;
        u32 size_high : 4;
        u32 address_mid : 4;

        static inline BufferDescriptor Create(void *buf, u64 buf_size, BufferMode mode) {
            return {
                .size_low = static_cast<u32>(buf_size),
                .address_low = static_cast<u32>(reinterpret_cast<u64>(buf)),
                .mode = static_cast<u32>(mode),
                .address_high = static_cast<u32>(reinterpret_cast<u64>(buf) >> 36),
                .size_high = static_cast<u32>(buf_size >> 32),
                .address_mid = static_cast<u32>(reinterpret_cast<u64>(buf) >> 32),                
            };
        }

    };

    struct SendStaticDescriptor {
        u32 index : 6;
        u32 address_high : 6;
        u32 address_mid : 4;
        u32 size : 16;
        u32 address_low;

        static inline SendStaticDescriptor Create(void *buf, u64 buf_size, u32 index) {
            return {
                .index = index,
                .address_high = static_cast<u32>(reinterpret_cast<u64>(buf) >> 36),
                .address_mid = static_cast<u32>(reinterpret_cast<u64>(buf) >> 32),
                .size = static_cast<u32>(buf_size),
                .address_low = static_cast<u32>(reinterpret_cast<u64>(buf)),               
            };
        }

    };

    struct ReceiveStaticDescriptor {
        u32 address_low;
        u32 address_high : 16;
        u32 size : 16;

        static inline ReceiveStaticDescriptor Create(void *buf, u64 buf_size) {
            return {
                .address_low = static_cast<u32>(reinterpret_cast<u64>(buf)),
                .address_high = static_cast<u32>(reinterpret_cast<u64>(buf) >> 32),
                .size = static_cast<u32>(buf_size),
            };
        }

    };

    struct CommandHeader {
        u32 command_type : 16;
        u32 send_static_count : 4;
        u32 send_buffer_count : 4;
        u32 receive_buffer_count : 4;
        u32 exchange_buffer_count : 4;
        u32 data_word_count : 10;
        u32 receive_static_type : 4;
        u32 pad : 6;
        u32 unused : 11;
        u32 has_special_header : 1;

        static constexpr u32 ReceiveStaticAutoType = 0xFF;

        static inline constexpr u32 MakeReceiveStaticType(u32 receive_static_count) {
            u32 type = 0;
            if(receive_static_count > 0) {
                type += 2;
                if(receive_static_count != ReceiveStaticAutoType) {
                    type += receive_static_count;
                }
            }
            return type;
        }

        static inline constexpr u32 GetReceiveStaticCount(u32 receive_static_type) {
            u32 count = 0;
            if(receive_static_type > 0) {
                if(receive_static_type == 2) {
                    count = ReceiveStaticAutoType;
                }
                else if(receive_static_type > 2) {
                    count = receive_static_type - 2;
                }
            }
            return count;
        }

    };

    struct CommandSpecialHeader {
        u32 send_process_id : 1;
        u32 copy_handle_count : 4;
        u32 move_handle_count : 4;
        u32 pad : 23;
    };

    struct DataHeader {
        u32 magic;
        u32 version;
        u32 value; // request ID for requests, result for responses
        u32 token;
    };

    constexpr u32 DataInHeaderMagic = 0x49434653;
    constexpr u32 DataOutHeaderMagic = 0x4F434653;

    struct DomainInDataHeader {
        u8 type;
        u8 in_object_count;
        u16 data_size;
        u32 object_id;
        u32 pad;
        u32 token;
    };

    struct DomainOutDataHeader {
        u32 out_object_count;
        u32 pad[3];
    };

    enum class ControlRequestId : u32 {
        ConvertCurrentObjectToDomain,
        CopyFromCurrentDomain,
        CloneCurrentObject,
        QueryPointerBufferSize,
        CloneCurrentObjectEx,
    };

    enum class BufferAttribute : u8 {
        In = BIO_BITMASK(0),
        Out = BIO_BITMASK(1),
        MapAlias = BIO_BITMASK(2),
        Pointer = BIO_BITMASK(3),
        FixedSize = BIO_BITMASK(4),
        AutoSelect = BIO_BITMASK(5),
        MapTransferAllowsNonSecure = BIO_BITMASK(6),
        MapTransferAllowsNonDevice = BIO_BITMASK(7),
    };

    BIO_ENUM_BIT_OPERATORS(BufferAttribute, u8)

    enum class CommandType : u32 {
        Invalid,
        LegacyRequest,
        Close,
        LegacyControl,
        Request,
        Control,
        RequestWithContext,
        ControlWithContext,
    };

    enum class DomainCommandType : u8 {
        Invalid,
        SendMessage,
        Close,
    };

    static constexpr u64 MaxHandleCount = 8;
    static constexpr u64 MaxObjectIdCount = 8;
    static constexpr u64 MaxBufferCount = 8;

    struct CommandInParameters {
        bool send_process_id;
        u64 process_id;
        u32 data_size;
        u8 *data_offset;
        u8 *data_words_offset;
        u8 *objects_offset;
        util::LinkedList<u32> copy_handles;
        util::LinkedList<u32> move_handles;
        util::LinkedList<u32> objects;
        util::LinkedList<u16> out_pointer_sizes;
    };

    struct CommandOutParameters {
        bool send_process_id;
        u64 process_id;
        u32 data_size;
        u8 *data_offset;
        u8 *data_words_offset;
        util::LinkedList<u32> copy_handles;
        util::LinkedList<u32> move_handles;
        util::LinkedList<u32> objects;
    };

    constexpr u32 NoOutProcessId = -1;

    struct CommandContext {
        SessionBase session_copy;
        CommandInParameters in;
        CommandOutParameters out;
        util::LinkedList<SendStaticDescriptor> send_statics;
        util::LinkedList<ReceiveStaticDescriptor> receive_statics;
        util::LinkedList<BufferDescriptor> send_buffers;
        util::LinkedList<BufferDescriptor> receive_buffers;
        util::LinkedList<BufferDescriptor> exchange_buffers;

        CommandContext(SessionBase session) : session_copy(session), in(), out(), send_statics(), receive_statics(), send_buffers(), receive_buffers(), exchange_buffers() {}

        template<HandleMode Mode>
        inline constexpr void AddInHandle(u32 handle) {
            switch(Mode) {
                case HandleMode::Copy: {
                    this->in.copy_handles.PushBack(handle);
                    break;
                }
                case HandleMode::Move: {
                    this->in.move_handles.PushBack(handle);
                    break;
                }
            }
        }

        inline constexpr void PushInObject(u32 object_id) {
            this->in.objects.PushBack(object_id);
        }

        inline void AddBuffer(void *buf, u64 buf_size, BufferAttribute attr) {
            const bool is_in = static_cast<bool>(attr & BufferAttribute::In);
            const bool is_out = static_cast<bool>(attr & BufferAttribute::Out);
            if(static_cast<bool>(attr & BufferAttribute::AutoSelect)) {
                u16 ptr_buf_size = 0;
                // TODO: pointer buffer size
                const bool buffer_in_static = (ptr_buf_size > 0) && (buf_size <= ptr_buf_size);
                if(is_in) {
                    if(buffer_in_static) {
                        auto send_buf_desc = BufferDescriptor::Create(nullptr, 0, BufferMode::Normal);
                        send_buffers.PushBack(send_buf_desc);
                        auto send_static_desc = SendStaticDescriptor::Create(buf, buf_size, send_statics.GetSize());
                        send_statics.PushBack(send_static_desc);
                    }
                    else {
                        auto send_buf_desc = BufferDescriptor::Create(buf, buf_size, BufferMode::Normal);
                        send_buffers.PushBack(send_buf_desc);
                        auto send_static_desc = SendStaticDescriptor::Create(nullptr, 0, send_statics.GetSize());
                        send_statics.PushBack(send_static_desc);
                    }
                }
                else if(is_out) {
                    if(buffer_in_static) {
                        auto receive_buf_desc = BufferDescriptor::Create(nullptr, 0, BufferMode::Normal);
                        receive_buffers.PushBack(receive_buf_desc);
                        auto receive_static_desc = ReceiveStaticDescriptor::Create(buf, buf_size);
                        receive_statics.PushBack(receive_static_desc);
                        in.out_pointer_sizes.PushBack(static_cast<u16>(buf_size));
                    }
                    else {
                        auto receive_buf_desc = BufferDescriptor::Create(buf, buf_size, BufferMode::Normal);
                        receive_buffers.PushBack(receive_buf_desc);
                        auto receive_static_desc = ReceiveStaticDescriptor::Create(nullptr, 0);
                        receive_statics.PushBack(receive_static_desc);
                        in.out_pointer_sizes.PushBack(0);
                    }
                }
            }
            else if(static_cast<bool>(attr & BufferAttribute::Pointer)) {
                if(is_in) {
                    auto send_static_desc = SendStaticDescriptor::Create(buf, buf_size, send_statics.GetSize());
                    send_statics.PushBack(send_static_desc);
                }
                else if(is_out) {
                    auto receive_static_desc = ReceiveStaticDescriptor::Create(buf, buf_size);
                    receive_statics.PushBack(receive_static_desc);
                    if(!static_cast<bool>(attr & BufferAttribute::FixedSize)) {
                        in.out_pointer_sizes.PushBack(static_cast<u16>(buf_size));
                    }
                }
            }
            else if(static_cast<bool>(attr & BufferAttribute::MapAlias)) {
                auto mode = BufferMode::Normal;
                if(static_cast<bool>(attr & BufferAttribute::MapTransferAllowsNonSecure)) {
                    mode = BufferMode::NonSecure;
                }
                else if(static_cast<bool>(attr & BufferAttribute::MapTransferAllowsNonDevice)) {
                    mode = BufferMode::NonDevice;
                }
                auto buf_desc = BufferDescriptor::Create(buf, buf_size, mode);
                if(is_in && is_out) {
                    exchange_buffers.PushBack(buf_desc);
                }
                else if(is_in) {
                    send_buffers.PushBack(buf_desc);
                }
                else if(is_out) {
                    receive_buffers.PushBack(buf_desc);
                }
            }
        };

        template<HandleMode Mode>
        inline constexpr u32 PopOutHandle() {
            switch(Mode) {
                case HandleMode::Copy: {
                    auto handle = this->out.copy_handles.Front();
                    this->out.copy_handles.PopFront();
                    return handle;
                }
                case HandleMode::Move: {
                    auto handle = this->out.move_handles.Front();
                    this->out.move_handles.PopFront();
                    return handle;
                }
            }
        }

        inline constexpr u32 PopOutObject() {
            auto object = this->out.objects.Front();
            this->out.objects.PopFront();
            return object;
        }

    };

    template<typename T>
    inline u8 *WriteSizedArrayToBuffer(u8 *buf, util::LinkedList<T> &array) {
        auto tmp_buf = buf;
        T tmp;
        auto it = array.Iterate();
        while(it.GetNext(tmp)) {
            *reinterpret_cast<T*>(tmp_buf) = tmp;
            tmp_buf += sizeof(T);
        }
        return tmp_buf;
    }

    template<typename T>
    inline u8 *ReadSizedArrayFromBuffer(u8 *buf, u32 count, util::LinkedList<T> &array) {
        auto tmp_buf = buf;
        if(count > 0) {
            array.Clear();
            for(u32 i = 0; i < count; i++) {
                array.PushBack(*reinterpret_cast<T*>(tmp_buf));
                tmp_buf += sizeof(T);
            }
        }
        return tmp_buf;
    }

    inline u8 *GetAlignedDataOffset(u8 *data_words_offset, u8 *base) {
        i64 data_offset = (data_words_offset - base + 15) &~ 15;
        return base + data_offset;
    }

    inline u8 *GetIpcBuffer() {
        return os::GetThreadLocalStorage()->ipc_buffer;
    }

}