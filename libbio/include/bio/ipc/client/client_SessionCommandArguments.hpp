
#pragma once
#include <bio/ipc/client/client_Session.hpp>

namespace bio::ipc::client {

    template<u32 Index>
    struct OutSession : public CommandArgument {

        Session &session;

        constexpr OutSession(Session &session_ref) : session(session_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    if(ctx.session_copy.IsDomain()) {
                        auto obj_id = InvalidObjectId;
                        if(ctx.HasOutObject<Index>()) {
                            ctx.GetOutObject<Index>(obj_id);
                            session = Session::CreateDomainFromParent(ctx.session_copy, obj_id);
                        }
                    }
                    else {
                        auto handle = InvalidHandle;
                        if(ctx.HasOutHandle<HandleMode::Move, Index>()) {
                            ctx.GetOutHandle<HandleMode::Move, Index>(handle);
                            session = Session::CreateFromHandle(handle);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<u32 Index, typename S>
    struct OutSessionObject : public CommandArgument {
        static_assert(IsSessionObject<S>, "Invalid session object");

        mem::SharedObject<S> &session_obj;

        constexpr OutSessionObject(mem::SharedObject<S> &session_obj_ref) : session_obj(session_obj_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    if(ctx.session_copy.IsDomain()) {
                        auto obj_id = InvalidObjectId;
                        if(ctx.HasOutObject<Index>()) {
                            ctx.GetOutObject<Index>(obj_id);
                            auto session = Session::CreateDomainFromParent(ctx.session_copy, obj_id);
                            mem::NewShared(session_obj, session); // Check result...?
                        }
                    }
                    else {
                        auto handle = InvalidHandle;
                        if(ctx.HasOutHandle<HandleMode::Move, Index>()) {
                            ctx.GetOutHandle<HandleMode::Move, Index>(handle);
                            auto session = Session::CreateFromHandle(handle);
                            mem::NewShared(session_obj, session); // Check result...?
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

}