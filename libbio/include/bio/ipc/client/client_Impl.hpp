    
#pragma once
#include <bio/ipc/ipc_Types.hpp>
#include <bio/util/util_Array.hpp>
#include <bio/util/util_Templates.hpp>

namespace bio::ipc::client {

    inline void WriteCommandOnTls(CommandContext &ctx, CommandType type, u32 data_size) {
        auto tls = os::GetThreadLocalStorage<u8>();
        auto header = reinterpret_cast<CommandHeader*>(tls);

        if(ctx.in.send_process_id) {
            data_size += sizeof(u64);
        }

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
        tls += sizeof(CommandHeader);

        if(special_header) {
            auto special_h = reinterpret_cast<CommandSpecialHeader*>(tls);
            special_h->send_process_id = ctx.in.send_process_id;
            special_h->copy_handle_count = static_cast<u32>(ctx.in.copy_handles.GetSize());
            special_h->move_handle_count = static_cast<u32>(ctx.in.move_handles.GetSize());
            special_h->pad = 0;
            tls += sizeof(CommandSpecialHeader);

            if(ctx.in.send_process_id) {
                tls += sizeof(u64);
            }

            tls = WriteSizedArrayToTls(tls, ctx.in.copy_handles);
            tls = WriteSizedArrayToTls(tls, ctx.in.move_handles);
        }

        tls = WriteSizedArrayToTls(tls, ctx.send_statics);
        tls = WriteSizedArrayToTls(tls, ctx.send_buffers);
        tls = WriteSizedArrayToTls(tls, ctx.receive_buffers);
        tls = WriteSizedArrayToTls(tls, ctx.exchange_buffers);
        ctx.in.data_words_offset = tls;
        tls += sizeof(u32) * header->data_word_count;
        tls = WriteSizedArrayToTls(tls, ctx.receive_statics);
    }

    inline void ReadCommandResponseFromTls(CommandContext &ctx) {
        auto tls = os::GetThreadLocalStorage<u8>();
        auto header = reinterpret_cast<CommandHeader*>(tls);
        tls += sizeof(CommandHeader);

        u32 copy_h_count = 0;
        u32 move_h_count = 0;
        ctx.out.process_id = NoOutProcessId;
        if(header->has_special_header) {
            auto special_header = reinterpret_cast<CommandSpecialHeader*>(tls);
            copy_h_count = special_header->copy_handle_count;
            move_h_count = special_header->move_handle_count;
            tls += sizeof(CommandSpecialHeader);
            if(special_header->send_process_id) {
                ctx.out.process_id = *reinterpret_cast<u64*>(tls);
                tls += sizeof(u64);
            }
        }

        tls = ReadSizedArrayFromTls(tls, copy_h_count, ctx.out.copy_handles);
        tls = ReadSizedArrayFromTls(tls, move_h_count, ctx.out.move_handles);

        tls += sizeof(SendStaticDescriptor) * header->send_static_count;
        ctx.out.data_words_offset = tls;
    }

    // Request

    constexpr i32 NoRequestId = -1;

    inline void WriteRequestCommandOnTls(CommandContext &ctx, i32 request_id, DomainCommandType domain_command) {
        auto tls = os::GetThreadLocalStorage<u8>();
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

        WriteCommandOnTls(ctx, CommandType::Request, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, tls);

        auto out_pointer_sizes = ctx.in.data_words_offset + out_pointer_sizes_offset;
        WriteSizedArrayToTls(out_pointer_sizes, ctx.in.out_pointer_sizes);

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

    inline Result ReadRequestCommandResponseFromTls(CommandContext &ctx) {
        auto tls = os::GetThreadLocalStorage<u8>();
        ReadCommandResponseFromTls(ctx);

        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, tls);
        auto header = reinterpret_cast<DataHeader*>(data_offset);

        if(ctx.session_copy.IsDomain()) {
            auto domain_header = reinterpret_cast<DomainOutDataHeader*>(data_offset);
            data_offset += sizeof(DomainOutDataHeader);
            auto objects_offset = data_offset + sizeof(DataHeader) + ctx.out.data_size;
            ReadSizedArrayFromTls(objects_offset, domain_header->out_object_count, ctx.out.objects);
            header = reinterpret_cast<DataHeader*>(data_offset);
        }

        data_offset += sizeof(DataHeader);
        BIO_RET_UNLESS(header->magic == DataOutHeaderMagic, result::ResultInvalidRequestCommandResponse);
        BIO_RES_TRY(header->value);
    
        ctx.out.data_offset = data_offset;
        return ResultSuccess;
    }

    // Control

    inline void WriteControlCommandOnTls(CommandContext &ctx, u32 request_id) {
        auto tls = os::GetThreadLocalStorage<u8>();
        u32 data_size = 16 + sizeof(DataHeader) + ctx.in.data_size;

        WriteCommandOnTls(ctx, CommandType::Control, data_size);
        auto data_offset = GetAlignedDataOffset(ctx.in.data_words_offset, tls);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        header->magic = DataInHeaderMagic;
        header->version = 0; // context?
        header->value = request_id;
        header->token = 0;
        data_offset += sizeof(DataHeader);
        ctx.in.data_offset = data_offset;
    }

    inline Result ReadControlCommandResponseFromTls(CommandContext &ctx) {
        auto tls = os::GetThreadLocalStorage<u8>();
        ReadCommandResponseFromTls(ctx);
        auto data_offset = GetAlignedDataOffset(ctx.out.data_words_offset, tls);

        auto header = reinterpret_cast<DataHeader*>(data_offset);
        data_offset += sizeof(DataHeader);
        BIO_RET_UNLESS(header->magic == DataOutHeaderMagic, result::ResultInvalidRequestCommandResponse);
        BIO_RES_TRY(header->value);

        ctx.out.data_offset = data_offset;
        return ResultSuccess;
    }

    // Close

    inline void WriteCloseCommandOnTls(CommandContext &ctx) {
        WriteCommandOnTls(ctx, CommandType::Close, 0);
    }

    enum class CommandState {
        BeforeHeaderInitialization,
        BeforeRequest,
        AfterRequest,
        AfterResponseParse,
    };

    struct CommandArgument {};

}