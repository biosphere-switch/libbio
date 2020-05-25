
#pragma once
#include <bio/ipc/server/server_Impl.hpp>

namespace bio::ipc::server {

    template<typename T>
    struct In : public CommandArgument {

        T &ref;

        constexpr In(T &ref) : ref(ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeCommandHandler: {
                    util::OffsetCalculator off(ctx.in.data_offset, ctx.in.data_size);
                    this->ref = off.GetNext<T>();
                    ctx.in.data_size = off.GetCurrentOffset();
                    break;
                }
                default:
                    break;
            }
        }
 
    };

    struct InProcessId : public CommandArgument {
        u64 &pid;

        constexpr InProcessId(u64 &pid_ref) : pid(pid_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeCommandHandler: {
                    this->pid = ctx.in.process_id;
                    break;
                }
                default:
                    break;
            }
        }

    };

    /*
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

    struct InSession : public CommandArgument {
        SessionBase session;

        constexpr InSession(SessionBase session) : session(session) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state)
            {
                case CommandState::BeforeHeaderInitialization: {
                    if(this->session.IsValid()) {
                        if(this->session.IsDomain()) {
                            ctx.AddInObject(this->session.GetObjectId());
                        }
                    }
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
                    ctx.AddBuffer(this->buf, this->buf_size, this->attr);
                    break;
                }
                default:
                    break;
            }
        }

    };
    */

    template<typename T>
    struct Out : public CommandArgument {

        T value;
        u64 offset;

        constexpr Out(T value) : value(value), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeResponseWrite: {
                    util::OffsetCalculator off(ctx.out.data_size);
                    this->offset = off.GetNextOffset<T>();
                    ctx.out.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::AfterResponseWrite: {
                    util::OffsetCalculator off(ctx.out.data_offset);
                    off.SetByOffset<T>(this->offset, this->value);
                    break;
                }
                default:
                    break;
            }
        }
 
    };

    /*
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

    template<HandleMode Mode, u32 Index>
    struct OutHandle : public CommandArgument {
        u32 &handle;

        constexpr OutHandle(u32 &handle_ref) : handle(handle_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    ctx.GetOutHandle<Mode, Index>(this->handle);
                    break;
                }
                default:
                    break;
            }
        }

    };
    */

}