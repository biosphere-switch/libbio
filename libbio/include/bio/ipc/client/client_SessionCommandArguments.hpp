
#pragma once
#include <bio/ipc/client/client_Session.hpp>

namespace bio::ipc::client {

    struct InSession : public CommandArgument {
        Session session;

        constexpr InSession(Session session) : session(session) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state)
            {
                case CommandState::BeforeHeaderInitialization: {
                    if(this->session.IsValid()) {
                        if(this->session.IsDomain()) {
                            ctx.PushInObject(this->session.GetObjectId());
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<typename S>
    struct InSessionObject : public CommandArgument {
        static_assert(IsSessionObject<S>, "Invalid session object");

        mem::SharedObject<S> &session_obj;

        constexpr InSessionObject(mem::SharedObject<S> &session_obj_ref) : session_obj(session_obj_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state)
            {
                case CommandState::BeforeHeaderInitialization: {
                    auto &session = this->session_obj->GetSession();
                    if(session.IsValid()) {
                        if(session.IsDomain()) {
                            ctx.PushInObject(session.GetObjectId());
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct OutSession : public CommandArgument {

        Session &session;

        constexpr OutSession(Session &session_ref) : session(session_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    if(ctx.session_copy.IsDomain()) {
                        auto object = ctx.PopOutObject();
                        session = Session::CreateDomainFromParent(ctx.session_copy, object);
                    }
                    else {
                        auto handle = ctx.PopOutHandle<HandleMode::Move>();
                        session = Session::CreateFromHandle(handle);
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<typename S>
    struct OutSessionObject : public CommandArgument {
        static_assert(IsSessionObject<S>, "Invalid session object");

        mem::SharedObject<S> &session_obj;

        constexpr OutSessionObject(mem::SharedObject<S> &session_obj_ref) : session_obj(session_obj_ref) {}

        inline constexpr void Process(CommandContext &ctx, CommandState state) {
            switch(state) {
                case CommandState::AfterResponseParse: {
                    if(ctx.session_copy.IsDomain()) {
                        auto object = ctx.PopOutObject();
                        auto session = Session::CreateDomainFromParent(ctx.session_copy, object);
                        mem::NewShared(session_obj, session); // Check result...?
                    }
                    else {
                        auto handle = ctx.PopOutHandle<HandleMode::Move>();
                        auto session = Session::CreateFromHandle(handle);
                        mem::NewShared(session_obj, session); // Check result...?
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

}