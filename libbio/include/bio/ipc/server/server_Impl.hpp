    
#pragma once
#include <bio/ipc/ipc_Types.hpp>
#include <bio/util/util_Array.hpp>
#include <bio/util/util_Templates.hpp>

namespace bio::ipc::server {

    inline void ReadCommandFromIpcBuffer(CommandContext &ctx, CommandType &out_type) {
        auto ipc_buf = GetIpcBuffer();
        auto header = reinterpret_cast<CommandHeader*>(ipc_buf);
        ipc_buf += sizeof(CommandHeader);

        out_type = static_cast<CommandType>(header->command_type);
        auto data_size = header->data_word_count * 4;
        ctx.in.data_size = data_size;

        if(header->has_special_header) {
            auto special_header = reinterpret_cast<CommandSpecialHeader*>(ipc_buf);
            ipc_buf += sizeof(CommandSpecialHeader);
            
            ctx.in.send_process_id = special_header->send_process_id;
            if(special_header->send_process_id) {
                ctx.in.process_id = *reinterpret_cast<u64*>(ipc_buf);
                ipc_buf += sizeof(u64);
            }

            ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, special_header->copy_handle_count, ctx.in.copy_handles);
            ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, special_header->move_handle_count, ctx.in.move_handles);
        }

        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, header->send_static_count, ctx.send_statics);
        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, header->send_buffer_count, ctx.send_buffers);
        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, header->receive_buffer_count, ctx.receive_buffers);
        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, header->exchange_buffer_count, ctx.exchange_buffers);
        ctx.in.data_words_offset = ipc_buf;
        ipc_buf += data_size;
        auto receive_static_count = CommandHeader::GetReceiveStaticCount(header->receive_static_type);
        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, receive_static_count, ctx.receive_statics);
    }

    inline void WriteCommandResponseOnIpcBuffer(CommandContext &ctx, CommandType type, u32 data_size) {
        auto ipc_buf = GetIpcBuffer();
        auto header = reinterpret_cast<CommandHeader*>(ipc_buf);
        ipc_buf += sizeof(CommandHeader);

        header->command_type = static_cast<u32>(type);
        header->send_static_count = static_cast<u32>(ctx.send_statics.GetSize());
        header->send_buffer_count = static_cast<u32>(ctx.send_buffers.GetSize());
        header->receive_buffer_count = static_cast<u32>(ctx.receive_buffers.GetSize());
        header->exchange_buffer_count = static_cast<u32>(ctx.exchange_buffers.GetSize());
        header->data_word_count = (data_size + 3) / 4;
        header->receive_static_type = CommandHeader::MakeReceiveStaticType(static_cast<u32>(ctx.receive_statics.GetSize()));
        header->pad = 0;
        header->unused = 0;

        const bool special_header = ctx.out.send_process_id || !ctx.out.copy_handles.IsEmpty() || !ctx.out.move_handles.IsEmpty();
        header->has_special_header = special_header;

        if(special_header) {
            auto special_h = reinterpret_cast<CommandSpecialHeader*>(ipc_buf);
            ipc_buf += sizeof(CommandSpecialHeader);

            special_h->send_process_id = ctx.out.send_process_id;
            special_h->copy_handle_count = static_cast<u32>(ctx.out.copy_handles.GetSize());
            special_h->move_handle_count = static_cast<u32>(ctx.out.move_handles.GetSize());
            special_h->pad = 0;

            if(ctx.out.send_process_id) {
                ipc_buf += sizeof(u64);
            }

            ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.out.copy_handles);
            ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.out.move_handles);
        }

        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.send_statics);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.send_buffers);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.receive_buffers);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.exchange_buffers);
        ctx.out.data_words_offset = ipc_buf;
        ipc_buf += header->data_word_count * 4;
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.receive_statics);
    }

    // Request

    inline Result ReadRequestCommandFromIpcBuffer(CommandContext &ctx, u32 &out_request_id) {
        auto ipc_buf = GetIpcBuffer();
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, ipc_buf);

        /*
        auto out_pointer_sizes = ctx.in.data_words_offset + out_pointer_sizes_offset;
        WriteSizedArrayToBuffer(out_pointer_sizes, ctx.in.out_pointer_sizes);
        */

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        data_offset += sizeof(DataHeader);

        /*
        if(ctx.session_copy.IsDomain()) {
            auto domain_header = reinterpret_cast<DomainInDataHeader*>(data_offset);
            u32 rest_data_size = sizeof(DataHeader) + ctx.in.data_size;
            domain_header->type = static_cast<u8>(domain_command);
            domain_header->in_object_count = static_cast<u8>(ctx.in.objects.GetSize());
            domain_header->data_size = static_cast<u16>(rest_data_size);
            domain_header->object_id = ctx.session_copy.object_id;
            domain_header->pad = 0;
            domain_header->token = 0; // context?
            data_offset += sizeof(DomainInDataHeader);
            ctx.in.objects_offset = data_offset + rest_data_size;
            header = reinterpret_cast<DataHeader*>(data_offset);
        }
        */

        BIO_RET_UNLESS(header->magic == DataInHeaderMagic, cmif::result::ResultInvalidInputHeader);
        out_request_id = header->value;

        ctx.in.data_offset = data_offset;
        ctx.in.data_size -= (16 + sizeof(DataHeader));
        return ResultSuccess;
    }

    inline void WriteRequestCommandResponseOnIpcBuffer(CommandContext &ctx, Result rc) {
        auto ipc_buf = GetIpcBuffer();
        u32 data_size = 16 + sizeof(DataHeader) + ctx.out.data_size;
        /*
        if(ctx.session_copy.IsDomain()) {
            data_size += sizeof(DomainInDataHeader) + sizeof(u32) * ctx.in.objects.GetSize();
        }
        */
        data_size = (data_size + 1) &~ 1;
        /*
        auto out_pointer_sizes_offset = data_size;
        data_size += sizeof(u16) * ctx.out.out_pointer_sizes.GetSize();
        */

        WriteCommandResponseOnIpcBuffer(ctx, CommandType::Request, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, ipc_buf);

        /*
        auto out_pointer_sizes = ctx.out.data_words_offset + out_pointer_sizes_offset;
        WriteSizedArrayToBuffer(out_pointer_sizes, ctx.out.out_pointer_sizes);
        */

        auto header = reinterpret_cast<DataHeader*>(data_offset);

        /*
        if(ctx.session_copy.IsDomain()) {
            auto domain_header = reinterpret_cast<DomainInDataHeader*>(data_offset);
            u32 rest_data_size = sizeof(DataHeader) + ctx.in.data_size;
            domain_header->type = static_cast<u8>(domain_command);
            domain_header->in_object_count = static_cast<u8>(ctx.in.objects.GetSize());
            domain_header->data_size = static_cast<u16>(rest_data_size);
            domain_header->object_id = ctx.session_copy.object_id;
            domain_header->pad = 0;
            domain_header->token = 0;
            data_offset += sizeof(DomainInDataHeader);
            ctx.in.objects_offset = data_offset + rest_data_size;
            header = reinterpret_cast<DataHeader*>(data_offset);
        }
        */

        header->magic = DataOutHeaderMagic;
        header->version = 0; // context?
        header->value = rc.GetValue();
        header->token = 0;
        data_offset += sizeof(DataHeader);

        ctx.out.data_offset = data_offset;
    }

    // Control

    inline Result ReadControlCommandFromIpcBuffer(CommandContext &ctx, u32 &out_request_id) {
        auto ipc_buf = GetIpcBuffer();
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, ipc_buf);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        data_offset += sizeof(DataHeader);

        BIO_RET_UNLESS(header->magic == DataInHeaderMagic, cmif::result::ResultInvalidInputHeader);
        out_request_id = header->value;

        ctx.in.data_offset = data_offset;
        ctx.in.data_size -= (16 + sizeof(DataHeader));
        return ResultSuccess;
    }

    inline void WriteControlCommandResponseOnIpcBuffer(CommandContext &ctx, Result rc) {
        auto ipc_buf = GetIpcBuffer();
        u32 data_size = 16 + sizeof(DataHeader) + ctx.out.data_size;
        data_size = (data_size + 1) &~ 1;

        WriteCommandResponseOnIpcBuffer(ctx, CommandType::Request, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, ipc_buf);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        header->magic = DataOutHeaderMagic;
        header->version = 0;
        header->value = rc.GetValue();
        header->token = 0;
        data_offset += sizeof(DataHeader);

        ctx.out.data_offset = data_offset;
    }

    // Close

    inline void WriteCloseCommandResponseOnIpcBuffer(CommandContext &ctx) {
        WriteCommandResponseOnIpcBuffer(ctx, CommandType::Close, 0);
    }

    enum class CommandState {
        BeforeCommandHandler,
        BeforeResponseWrite,
        AfterResponseWrite,
    };

    struct CommandArgument {};

}