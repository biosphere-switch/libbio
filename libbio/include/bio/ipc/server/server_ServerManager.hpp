
#pragma once
#include <bio/ipc/server/server_MitmQuery.hpp>
#include <bio/ipc/server/server_Results.hpp>
#include <bio/util/util_List.hpp>

namespace bio::ipc::server {

    enum class WaitHandleType {
        Server,
        Session,
    };

    struct WaitHandle {
        u32 handle;
        WaitHandleType type;
    };

    class ServerObject {

        private:
            using CallHandlerFunction = Result(*)(Server*, u32, CommandContext&);
            using NewServerFunction = Result(*)(Server*&);
            using DeleteServerFunction = void(*)(Server*);

            template<typename S>
            static inline Result CallHandler(Server *server, u32 request_id, CommandContext &ctx) {
                const auto handlers = ServerCommandHandlers<S>;
                for(u32 i = 0; i < ServerCommandHandleCount<S>; i++) {
                    const auto &handler = handlers[i];
                    if(handler.request_id == request_id) {
                        return (reinterpret_cast<S*>(server)->*handler.fn)(ctx);
                    }
                }
                return 0xF601; // TODO: proper result?
            }

            template<typename S, typename ...SArgs>
            static inline Result NewServer(Server *&out_server, SArgs &&...s_args) {
                S *server;
                BIO_RES_TRY(mem::New(server, s_args...));

                out_server = reinterpret_cast<Server*>(server);
                return ResultSuccess;
            }

            template<typename S>
            static inline void DeleteServer(Server *server) {
                if(server != nullptr) {
                    mem::Delete<S>(reinterpret_cast<S*>(server));
                }
            }

        private:
            Server *server;
            CallHandlerFunction handler_fn;
            DeleteServerFunction delete_fn;
            service::sm::ServiceName service_name;
            bool is_mitm;
            util::SizedArray<WaitHandle, 0x40> handles;
            util::SizedArray<u32, 0x40> fwd_handles;

            Result HandleRequestCommand(u32 request_id, CommandContext &ctx) {
                return this->handler_fn(this->server, request_id, ctx);
            }

        public:
            static constexpr i32 InvalidIndex = -1;

        public:
            ServerObject(Server *server, u32 handle, WaitHandleType handle_type, bool is_mitm, service::sm::ServiceName name, CallHandlerFunction handler_fn, DeleteServerFunction delete_fn) : server(server), handler_fn(handler_fn), delete_fn(delete_fn), service_name(name), is_mitm(is_mitm) {
                this->handles.Push({ handle, handle_type });
                this->fwd_handles.Push(InvalidHandle);
            }

            ~ServerObject() {
                this->delete_fn(this->server);
                this->server = nullptr;
                // TODO: disposing, unregister service/uninstall mitm?
            }

            util::SizedArray<WaitHandle, 0x40> &GetHandles() {
                return this->handles;
            }

            Result ProcessSessionHandle(i32 index) {
                // AKA process a request from a session.
                auto &handle = this->handles.GetAt(index);
                auto &fwd_handle = this->fwd_handles.GetAt(index);

                i32 tmp_idx;
                BIO_RES_TRY(svc::ReplyAndReceive(tmp_idx, &handle.handle, 1, 0, svc::IndefiniteWait));

                u8 ipc_buf_backup[0x100];
                if(this->is_mitm) {
                    auto ipc_buf = GetIpcBuffer();
                    mem::Copy(ipc_buf_backup, ipc_buf, sizeof(ipc_buf_backup));
                }

                auto should_close_session = false;
                ipc::SessionBase base(handle.handle);

                ipc::CommandContext ctx(base);
                auto type = ipc::CommandType::Invalid;
                ReadCommandFromIpcBuffer(ctx, type);

                switch(type) {
                    case ipc::CommandType::Request: {
                        u32 rq_id;
                        auto rc = ReadRequestCommandFromIpcBuffer(ctx, rq_id);
                        if(rc.IsSuccess()) {
                            auto rc = this->HandleRequestCommand(rq_id, ctx);
                            if(this->is_mitm) {
                                if((rc == 0xF601) || (rc == service::sm::result::ResultAtmosphereMitmShouldForwardToSession)) {
                                    // Copy back temp TLS, and let the original session take care of the command
                                    auto ipc_buf = GetIpcBuffer();
                                    mem::Copy(ipc_buf, ipc_buf_backup, sizeof(ipc_buf_backup));
                                    BIO_RES_TRY(svc::SendSyncRequest(fwd_handle));
                                }
                            }
                        }
                        else {
                            WriteRequestCommandResponseOnIpcBuffer(ctx, rc);
                        }
                        break;
                    }
                    case ipc::CommandType::Close: {
                        should_close_session = true;
                        WriteCommandResponseOnIpcBuffer(ctx, CommandType::Close, 0);
                        break;
                    }
                    default: {
                        WriteRequestCommandResponseOnIpcBuffer(ctx, 0xAAAA);
                        break;
                    }
                }

                BIO_RES_TRY_EXCEPT(svc::ReplyAndReceive(tmp_idx, &handle.handle, 0, handle.handle, 0), os::result::ResultTimeOut);

                if(should_close_session) {
                    svc::CloseHandle(handle.handle);
                    this->handles.PopAt(index);
                }

                return ResultSuccess;
            }

            Result ProcessServerHandle(i32 index) {
                // AKA add/accept connection with a new session.
                auto &handle = this->handles.GetAt(index);

                u32 new_handle = 0;
                BIO_RES_TRY(svc::AcceptSession(new_handle, handle.handle));

                if(this->handles.IsFull()) {
                    svc::CloseHandle(new_handle);
                    return result::ResultHandleTableFull;
                }

                this->handles.Push({ new_handle, WaitHandleType::Session });

                u32 fwd_handle = InvalidHandle;
                if(this->is_mitm) {
                    service::sm::MitmProcessInfo info;

                    service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                    BIO_RES_TRY(sm);
                    BIO_RES_TRY(service::sm::UserNamedPortSession->AtmosphereAcknowledgeMitmSession(this->service_name, info, fwd_handle));
                }

                this->fwd_handles.Push(fwd_handle);

                return ResultSuccess;
            }

            i32 GetHandleIndex(u32 handle) {
                for(u32 i = 0; i < this->handles.GetSize(); i++) {
                    auto &this_handle = this->handles.GetAt(i);
                    if(handle == this_handle.handle) {
                        return i;
                    }
                }
                return InvalidIndex;
            }

            Result ProcessSignaledHandle(u32 index) {
                auto &handle = this->handles.GetAt(index);
                switch(handle.type) {
                    case WaitHandleType::Server: {
                        BIO_RES_TRY(this->ProcessServerHandle(index));
                        break;
                    }
                    case WaitHandleType::Session: {
                        BIO_RES_TRY(this->ProcessSessionHandle(index));
                        break;
                    }
                }

                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            static inline Result Create(ServerObject *&out_obj, u32 handle, WaitHandleType handle_type, bool is_mitm, service::sm::ServiceName name, SArgs &&...s_args) {
                Server *server;
                BIO_RES_TRY(NewServer<S>(server, s_args...));

                BIO_RES_TRY(mem::New(out_obj, server, handle, handle_type, is_mitm, name, &CallHandler<S>, &DeleteServer<S>));

                return ResultSuccess;
            }

    };

    class ServerManager {

        private:
            util::LinkedList<ServerObject*> servers;
            util::SizedArray<u32, 0x40> wait_handles;

            static Result RegisterMitmQuerySession(u32 mitm_query_handle, ShouldMitmFunction fn);

            void PrepareWaitHandles() {
                this->wait_handles.Clear();
                for(u32 i = 0; i < this->servers.GetSize(); i++) {
                    auto &server = this->servers.GetAt(i);
                    auto &server_handles = server->GetHandles();
                    for(u32 j = 0; j < server_handles.GetSize(); j++) {
                        auto &server_handle = server_handles.GetAt(j);
                        this->wait_handles.Push(server_handle.handle);
                    }
                }
            }

        public:
            template<typename S, typename ...SArgs>
            Result RegisterObject(u32 handle, WaitHandleType handle_type, bool is_mitm, service::sm::ServiceName name, SArgs &&...s_args) {
                static_assert(IsServer<S>, "Must be a Server type");

                ServerObject *obj;
                BIO_RES_TRY(ServerObject::Create<S>(obj, handle, handle_type, is_mitm, name, s_args...));
                BIO_RES_TRY(this->servers.PushBack(obj));
                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            Result RegisterServer(u32 handle, SArgs &&...s_args) {
                static_assert(IsServer<S>, "Must be a Server type");

                return this->RegisterObject<S>(handle, WaitHandleType::Server, false, service::sm::InvalidServiceName, s_args...);
            }

            template<typename S, typename ...SArgs>
            Result RegisterSession(u32 handle, SArgs &&...s_args) {
                static_assert(IsServer<S>, "Must be a Server type");

                return this->RegisterObject<S>(handle, WaitHandleType::Session, false, service::sm::InvalidServiceName, s_args...);
            }

            template<typename S, typename ...SArgs>
            Result RegisterServiceServer(SArgs &&...s_args) {
                static_assert(IsService<S>, "Must be a Service type");

                const auto name = service::sm::ServiceName::Encode(S::GetName());
                u32 handle;
                service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                BIO_RES_TRY(sm);
                BIO_RES_TRY(service::sm::UserNamedPortSession->RegisterService(name, false, S::GetMaxSessions(), handle));

                BIO_RES_TRY(this->RegisterObject<S>(handle, WaitHandleType::Server, false, name, s_args...));
                
                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            Result RegisterMitmServiceServer(SArgs &&...s_args) {
                static_assert(IsMitmService<S>, "Must be a MitmService type");

                const auto name = service::sm::ServiceName::Encode(S::GetName());
                u32 handle;
                u32 mitm_query_handle;
                service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                BIO_RES_TRY(sm);
                BIO_RES_TRY(service::sm::UserNamedPortSession->AtmosphereInstallMitm(name, handle, mitm_query_handle));

                BIO_RES_TRY(this->RegisterMitmQuerySession(mitm_query_handle, &S::ShouldMitm));
                BIO_RES_TRY(this->RegisterObject<S>(handle, WaitHandleType::Server, true, name, s_args...));

                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            Result RegisterNamedPortServer(SArgs &&...s_args) {
                static_assert(IsNamedPort<S>, "Must be a NamedPort type");

                u32 handle;
                BIO_RES_TRY(svc::ManageNamedPort(handle, S::GetPortName(), S::GetMaxSessions()));
                BIO_RES_TRY(this->RegisterServer<S>(handle, s_args...));

                return ResultSuccess;
            }

            Result Process() {
                this->PrepareWaitHandles();

                auto out_idx = ServerObject::InvalidIndex;
                BIO_RES_TRY(svc::WaitSynchronization(out_idx, this->wait_handles.Get(), this->wait_handles.GetSize(), svc::IndefiniteWait));

                if((out_idx >= 0) && (out_idx < this->wait_handles.GetSize())) {
                    auto &signaled_handle = this->wait_handles.GetAt(out_idx);

                    ServerObject *server;
                    auto it = this->servers.Iterate();
                    while(it.GetNext(server)) {
                        auto idx = server->GetHandleIndex(signaled_handle);
                        if(idx != ServerObject::InvalidIndex) {
                            // The handle belongs to the server object.
                            server->ProcessSignaledHandle(idx);
                            break;
                        }
                    }
                };

                return ResultSuccess;
            }

            void LoopProcess() {
                while(true) {
                    auto rc = this->Process();
                    if(rc == os::result::ResultOperationCancelled) {
                        break;
                    }
                }
            }

    };

}