
#pragma once
#include <bio/ipc/client/client_Impl.hpp>

namespace bio::ipc::client {

    template<typename T>
    struct In : public CommandArgument {

        T value;
        u64 offset;

        constexpr In(T val) : value(val), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    util::OffsetCalculator off(ctx.in.data_size);
                    this->offset = off.GetNextOffset<T>();
                    ctx.in.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::BeforeRequest: {
                    util::OffsetCalculator off(ctx.in.data_offset);
                    off.SetByOffset<T>(this->offset, this->value);
                    break;
                }
                default:
                    break;
            }
        }
 
    };

    struct InProcessId : public CommandArgument {

        u64 pid_value;
        u64 offset;

        constexpr InProcessId(u64 value = 0) : pid_value(value), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    ctx.in.send_process_id = true;
                    util::OffsetCalculator off(ctx.in.data_size);
                    this->offset = off.GetNextOffset<u64>();
                    ctx.in.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::BeforeRequest: {
                    util::OffsetCalculator off(ctx.in.data_offset);
                    off.SetByOffset(this->offset, this->pid_value);
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<HandleMode Mode>
    struct InHandle : public CommandArgument {
        u32 handle;

        constexpr InHandle(u32 handle) : handle(handle) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    ctx.AddInHandle<Mode>(this->handle);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct Buffer : public CommandArgument {
        void *buf;
        u64 buf_size;
        BufferAttribute attr;

        constexpr Buffer(void *buf, u64 buf_size, BufferAttribute attr) : buf(buf), buf_size(buf_size), attr(attr) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    if(this->buf != nullptr) {
                        if(this->buf_size > 0) {
                            ctx.AddBuffer(this->buf, this->buf_size, this->attr);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<typename T>
    struct Out : public CommandArgument {
        T &value;
        u64 offset;

        constexpr Out(T &ref) : value(ref), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterRequest: {
                    util::OffsetCalculator off(ctx.out.data_size);
                    this->offset = off.GetNextOffset<T>();
                    ctx.out.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::AfterResponseParse: {
                    util::OffsetCalculator off(ctx.out.data_offset);
                    this->value = off.GetByOffset<T>(this->offset);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct OutProcessId : public CommandArgument {
        u64 &pid;

        constexpr OutProcessId(u64 &pid_ref) : pid(pid_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    this->pid = ctx.out.process_id;
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<HandleMode Mode>
    struct OutHandle : public CommandArgument {
        u32 &handle;

        constexpr OutHandle(u32 &handle_ref) : handle(handle_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    this->handle = ctx.PopOutHandle<Mode>();
                    break;
                }
                default:
                    break;
            }
        }

    };

}