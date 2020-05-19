
#pragma once
#include <bio/ipc/client/client_Impl.hpp>

namespace bio::ipc::client {

    template<typename T>
    struct In : public CommandArgument {

        T value;
        u64 offset;

        constexpr In(T type) : value(type), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    OffsetCalculator off(ctx.in.data_size);
                    this->offset = off.GetNextOffset<T>();
                    ctx.in.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::BeforeRequest: {
                    OffsetCalculator off(ctx.in.data_offset);
                    off.SetByOffset<T>(this->offset, this->value);
                    break;
                }
                default:
                    break;
            }
        }
 
    };

    struct InProcessId : public CommandArgument {

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::BeforeHeaderInitialization: {
                    ctx.in.send_process_id = true;
                    // Add an u64 placeholder for the process id
                    OffsetCalculator off(ctx.in.data_size);
                    off.IncrementOffset<u64>();
                    ctx.in.data_size = off.GetCurrentOffset();
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

    template<typename T>
    struct Out : public CommandArgument {

        T &value;
        u64 offset;

        constexpr Out(T &ref) : value(ref), offset(0) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterRequest: {
                    OffsetCalculator off(ctx.out.data_size);
                    this->offset = off.GetNextOffset<T>();
                    ctx.out.data_size = off.GetCurrentOffset();
                    break;
                }
                case CommandState::AfterResponseParse: {
                    OffsetCalculator off(ctx.out.data_offset);
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

    /*
    template<u32 OIndex, bool AutoClear>
    class OutEvent : public CommandArgument
    {
        public:
            OutEvent(std::shared_ptr<os::Event> &out_ev) : event(out_ev), idx(OIndex), auto_cl(AutoClear)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 4:
                        if(idx < data.out_hs_size)
                        {
                            u32 handle = data.out_hs[idx];
                            event = std::move(os::Event::Open(handle, auto_cl));
                        }
                        break;
                }
            }

        private:
            u32 idx;
            bool auto_cl;
            std::shared_ptr<os::Event> &event;
    };
    */

}