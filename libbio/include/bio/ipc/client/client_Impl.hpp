    
#pragma once
#include <bio/ipc/ipc_Types.hpp>
#include <bio/util/util_Array.hpp>
#include <bio/util/util_Templates.hpp>

namespace bio::ipc::client {

    inline void WriteCommandOnIpcBuffer(CommandContext &ctx, CommandType type, u32 data_size) {
        auto ipc_buf = GetIpcBuffer();
        auto header = reinterpret_cast<CommandHeader*>(ipc_buf);

        header->command_type = static_cast<u32>(type);
        header->send_static_count = static_cast<u32>(ctx.send_statics.GetSize());
        header->send_buffer_count = static_cast<u32>(ctx.send_buffers.GetSize());
        header->receive_buffer_count = static_cast<u32>(ctx.receive_buffers.GetSize());
        header->exchange_buffer_count = static_cast<u32>(ctx.exchange_buffers.GetSize());
        header->data_word_count = (data_size + 3) / 4;
        header->receive_static_type = CommandHeader::MakeReceiveStaticType(static_cast<u32>(ctx.receive_statics.GetSize()));
        header->pad = 0;
        header->unused = 0;
        
        const bool special_header = ctx.in.send_process_id || !ctx.in.copy_handles.IsEmpty() || !ctx.in.move_handles.IsEmpty();
        header->has_special_header = static_cast<u32>(special_header);
        ipc_buf += sizeof(CommandHeader);

        if(special_header) {
            auto special_h = reinterpret_cast<CommandSpecialHeader*>(ipc_buf);
            special_h->send_process_id = ctx.in.send_process_id;
            special_h->copy_handle_count = static_cast<u32>(ctx.in.copy_handles.GetSize());
            special_h->move_handle_count = static_cast<u32>(ctx.in.move_handles.GetSize());
            special_h->pad = 0;
            ipc_buf += sizeof(CommandSpecialHeader);

            if(ctx.in.send_process_id) {
                ipc_buf += sizeof(u64);
            }

            ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.in.copy_handles);
            ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.in.move_handles);
        }

        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.send_statics);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.send_buffers);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.receive_buffers);
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.exchange_buffers);
        ctx.in.data_words_offset = ipc_buf;
        ipc_buf += sizeof(u32) * header->data_word_count;
        ipc_buf = WriteSizedArrayToBuffer(ipc_buf, ctx.receive_statics);
    }

    inline void ReadCommandResponseFromIpcBuffer(CommandContext &ctx) {
        auto ipc_buf = GetIpcBuffer();
        auto header = reinterpret_cast<CommandHeader*>(ipc_buf);
        ipc_buf += sizeof(CommandHeader);

        u32 copy_h_count = 0;
        u32 move_h_count = 0;
        ctx.out.process_id = NoOutProcessId;
        if(header->has_special_header) {
            auto special_header = reinterpret_cast<CommandSpecialHeader*>(ipc_buf);
            copy_h_count = special_header->copy_handle_count;
            move_h_count = special_header->move_handle_count;
            ipc_buf += sizeof(CommandSpecialHeader);
            if(special_header->send_process_id) {
                ctx.out.process_id = *reinterpret_cast<u64*>(ipc_buf);
                ipc_buf += sizeof(u64);
            }
        }

        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, copy_h_count, ctx.out.copy_handles);
        ipc_buf = ReadSizedArrayFromBuffer(ipc_buf, move_h_count, ctx.out.move_handles);

        ipc_buf += sizeof(SendStaticDescriptor) * header->send_static_count;
        ctx.out.data_words_offset = ipc_buf;
    }

    // Request

    constexpr i32 NoRequestId = -1;

    inline void WriteRequestCommandOnIpcBuffer(CommandContext &ctx, i32 request_id, DomainCommandType domain_command) {
        auto ipc_buf = GetIpcBuffer();
        const bool has_data_header = request_id != NoRequestId;

        u32 data_size = 16 + ctx.in.data_size;
        if(has_data_header) {
            data_size += sizeof(DataHeader);
        }
        if(ctx.session_copy.IsDomain()) {
            data_size += sizeof(DomainInDataHeader) + sizeof(u32) * ctx.in.objects.GetSize();
        }
        data_size = (data_size + 1) &~ 1;
        auto out_pointer_sizes_offset = data_size;
        data_size += sizeof(u16) * ctx.in.out_pointer_sizes.GetSize();

        WriteCommandOnIpcBuffer(ctx, CommandType::Request, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, ipc_buf);

        auto out_pointer_sizes = ctx.in.data_words_offset + out_pointer_sizes_offset;
        WriteSizedArrayToBuffer(out_pointer_sizes, ctx.in.out_pointer_sizes);

        auto header = reinterpret_cast<DataHeader*>(data_offset);

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

        if(has_data_header) {
            header->magic = DataInHeaderMagic;
            header->version = 0; // context?
            header->value = request_id;
            header->token = 0;
            data_offset += sizeof(DataHeader);
        }

        ctx.in.data_offset = data_offset;
    }

    inline Result ReadRequestCommandResponseFromIpcBuffer(CommandContext &ctx) {
        auto ipc_buf = GetIpcBuffer();
        ReadCommandResponseFromIpcBuffer(ctx);

        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, ipc_buf);
        auto header = reinterpret_cast<DataHeader*>(data_offset);

        if(ctx.session_copy.IsDomain()) {
            auto domain_header = reinterpret_cast<DomainOutDataHeader*>(data_offset);
            data_offset += sizeof(DomainOutDataHeader);
            auto objects_offset = data_offset + sizeof(DataHeader) + ctx.out.data_size;
            ReadSizedArrayFromBuffer(objects_offset, domain_header->out_object_count, ctx.out.objects);
            header = reinterpret_cast<DataHeader*>(data_offset);
        }

        data_offset += sizeof(DataHeader);
        BIO_RET_UNLESS(header->magic == DataOutHeaderMagic, cmif::result::ResultInvalidOutputHeader);
        BIO_RES_TRY(header->value);
    
        ctx.out.data_offset = data_offset;
        return ResultSuccess;
    }

    // Control

    inline void WriteControlCommandOnIpcBuffer(CommandContext &ctx, ControlRequestId request_id) {
        auto ipc_buf = GetIpcBuffer();
        u32 data_size = 16 + sizeof(DataHeader) + ctx.in.data_size;

        WriteCommandOnIpcBuffer(ctx, CommandType::Control, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, ipc_buf);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        header->magic = DataInHeaderMagic;
        header->version = 0; // context?
        header->value = static_cast<u32>(request_id);
        header->token = 0;
        data_offset += sizeof(DataHeader);
        ctx.in.data_offset = data_offset;
    }

    inline Result ReadControlCommandResponseFromIpcBuffer(CommandContext &ctx) {
        auto ipc_buf = GetIpcBuffer();
        ReadCommandResponseFromIpcBuffer(ctx);
        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, ipc_buf);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        data_offset += sizeof(DataHeader);
        BIO_RET_UNLESS(header->magic == DataOutHeaderMagic, cmif::result::ResultInvalidOutputHeader);
        BIO_RES_TRY(header->value);

        ctx.out.data_offset = data_offset;
        return ResultSuccess;
    }

    // Close

    inline void WriteCloseCommandOnIpcBuffer(CommandContext &ctx) {
        WriteCommandOnIpcBuffer(ctx, CommandType::Close, 0);
    }

    enum class CommandState {
        BeforeHeaderInitialization,
        BeforeRequest,
        AfterRequest,
        AfterResponseParse,
    };

    struct CommandArgument {};

}