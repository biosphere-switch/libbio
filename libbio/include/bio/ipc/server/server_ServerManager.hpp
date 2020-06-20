
#pragma once
#include <bio/ipc/server/server_MitmQuery.hpp>
#include <bio/os/os_Wait.hpp>
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

    using NewServerFunction = Result(*)(Server*&);

    using DeleteServerFunction = void(*)(Server*);

    struct ServerContainer {
        Server *server;
        NewServerFunction new_fn;
        DeleteServerFunction delete_fn;
        WaitHandle handle;
        u32 forward_handle;
        bool is_mitm_service;
        service::sm::ServiceName service_name;

        inline constexpr bool IsMitmService() {
            return this->is_mitm_service;
        }

        inline constexpr bool IsMitmSession() {
            return this->forward_handle != InvalidHandle;
        }

        inline constexpr bool IsService() {
            return this->service_name.value != service::sm::InvalidServiceName.value;
        }

        inline void Delete() {
            this->delete_fn(this->server);
            if(this->IsMitmService()) {
                // TODO: uninstall mitm
            }
            else if(this->IsService()) {
                // TODO: unregister service
            }
            else {
                // Session handle, just close it
                svc::CloseHandle(this->handle.handle);
            }
        }

    };

    class ServerObject {

        private:
            using CallHandlerFunction = Result(*)(Server*, u32, CommandContext&);

            template<typename S>
            static inline Result CallHandler(Server *server, u32 request_id, CommandContext &ctx) {
                const auto handlers = ServerCommandHandlers<S>;
                for(u32 i = 0; i < ServerCommandHandleCount<S>; i++) {
                    const auto &handler = handlers[i];
                    if(handler.request_id == request_id) {
                        (reinterpret_cast<S*>(server)->*handler.fn)(ctx);
                        return ResultSuccess;
                    }
                }
                return cmif::result::ResultInvalidRequestId;
            }

            template<typename S>
            static inline Result NewServer(Server *&out_server) {
                S *server;
                BIO_RES_TRY(mem::New(server));

                out_server = reinterpret_cast<Server*>(server);
                return ResultSuccess;
            }

            template<typename S>
            static inline void DeleteServer(Server *server) {
                if(server != nullptr) {
                    mem::Delete<S>(reinterpret_cast<S*>(server));
                }
            }

            Result AddSession(ServerContainer &base_server, Server *server, u32 session_handle, u32 forward_handle) {
                ServerContainer new_server = {};
                new_server.server = server;
                new_server.new_fn = base_server.new_fn;
                new_server.delete_fn = base_server.delete_fn;
                new_server.is_mitm_service = false;
                new_server.handle = { session_handle, WaitHandleType::Session };
                new_server.service_name = service::sm::InvalidServiceName;
                new_server.forward_handle = forward_handle;
                this->servers.Push(new_server);
                return ResultSuccess;
            }
            
            Result AddNewSession(ServerContainer &base_server, u32 session_handle, u32 forward_handle) {
                Server *server;
                BIO_RES_TRY(base_server.new_fn(server));
                BIO_RES_TRY(this->AddSession(base_server, server, session_handle, forward_handle));
                
                return ResultSuccess;
            }

            // Control request implementations

            Result CloneCurrentObjectImpl(ServerContainer &server, CommandContext &ctx, u32 &out_handle) {
                u32 server_handle;
                u32 client_handle;
                BIO_RES_TRY(svc::CreateSession(server_handle, client_handle, false, 0));

                u32 forward_handle = InvalidHandle;
                if(server.IsMitmSession()) {
                    client::Session cloned_session;
                    auto forward_session = client::Session::CreateFromHandle(server.forward_handle);
                    BIO_RES_TRY(forward_session.CloneCurrentObject(cloned_session));

                    forward_handle = cloned_session.handle;
                }
                BIO_RES_TRY(this->AddSession(server, server.server, server_handle, forward_handle));
                out_handle = client_handle;
                return ResultSuccess;
            }

        private:
            CallHandlerFunction handler_fn;
            util::SizedArray<ServerContainer, os::MaxWaitObjectCount> servers;

        public:
            static constexpr i32 InvalidIndex = -1;

        public:
            ServerObject(CallHandlerFunction handler_fn, ServerContainer server) : handler_fn(handler_fn) {
                this->servers.Push(server);
            }

            ~ServerObject() {
                for(u32 i = 0; i < this->servers.GetSize(); i++) {
                    auto &server = this->servers.GetAt(i);
                    server.Delete();
                }
                this->servers.Clear();
            }

            util::SizedArray<ServerContainer, os::MaxWaitObjectCount> &GetServers() {
                return this->servers;
            }

            Result ProcessSessionHandle(i32 index, ServerContainer &server) {
                // AKA process a request from a session.
                i32 tmp_idx;
                BIO_RES_TRY(svc::ReplyAndReceive(tmp_idx, &server.handle.handle, 1, InvalidHandle, svc::IndefiniteWait));

                u8 ipc_buf_backup[0x100];
                if(server.IsMitmSession()) {
                    auto ipc_buf = GetIpcBuffer();
                    mem::Copy(ipc_buf_backup, ipc_buf, sizeof(ipc_buf_backup));
                }

                auto should_close_session = false;
                ipc::SessionBase base(server.handle.handle);

                ipc::CommandContext ctx(base);
                auto type = ipc::CommandType::Invalid;
                ReadCommandFromIpcBuffer(ctx, type);

                switch(type) {
                    case ipc::CommandType::Request: {
                        auto write_err_response = true;
                        u32 rq_id;
                        auto rc = ReadRequestCommandFromIpcBuffer(ctx, rq_id);
                        if(rc.IsSuccess()) {
                            write_err_response = false;
                            auto rc = this->handler_fn(server.server, rq_id, ctx);
                            if(rc.IsFailure()) {
                                write_err_response = true;
                                if(server.IsMitmSession()) {
                                    if((rc == cmif::result::ResultInvalidRequestId) || (rc == service::sm::result::ResultAtmosphereMitmShouldForwardToSession)) {
                                        // Copy back temp TLS, and let the original session take care of the command (if we weren't implementing it or explicitly asked for it to be forwarded)
                                        auto ipc_buf = GetIpcBuffer();
                                        mem::Copy(ipc_buf, ipc_buf_backup, sizeof(ipc_buf_backup));
                                        BIO_RES_TRY(svc::SendSyncRequest(server.forward_handle));
                                        write_err_response = false;
                                    }
                                }
                            }
                        }
                        if(write_err_response) {
                            // An error happened when running the request command - respond with it.
                            WriteRequestCommandResponseOnIpcBuffer(ctx, rc);
                        }
                        break;
                    }
                    case ipc::CommandType::Close: {
                        should_close_session = true;
                        WriteCloseCommandResponseOnIpcBuffer(ctx);
                        break;
                    }
                    case ipc::CommandType::Control: {
                        u32 rq_id;
                        auto rc = ReadControlCommandFromIpcBuffer(ctx, rq_id);
                        if(rc.IsSuccess()) {
                            switch(static_cast<ControlRequestId>(rq_id)) {
                                case ControlRequestId::CloneCurrentObject:
                                case ControlRequestId::CloneCurrentObjectEx: {
                                    // Note: the *Ex command just sends an unused u32 in raw data, so we'll ignore it.
                                    u32 cloned_handle;
                                    rc = this->CloneCurrentObjectImpl(server, ctx, cloned_handle);
                                    Server::RequestCommandEnd(ctx, rc, OutHandle<HandleMode::Move>(cloned_handle));
                                    break;
                                }
                                default: {
                                    rc = cmif::result::ResultInvalidRequestId;
                                    break;
                                }
                            }
                        }
                        WriteControlCommandResponseOnIpcBuffer(ctx, rc);
                        break;
                    }
                    default: {
                        // TODO: is this a proper result to return?
                        WriteCommandResponseOnIpcBuffer(ctx, CommandType::Invalid, cmif::result::ResultInvalidInputHeader);
                    }
                }

                BIO_RES_TRY_EXCEPT(svc::ReplyAndReceive(tmp_idx, &server.handle.handle, 0, server.handle.handle, 0), os::result::ResultTimeOut);

                if(should_close_session) {
                    server.Delete();
                    this->servers.PopAt(index);
                }

                return ResultSuccess;
            }

            Result ProcessServerHandle(i32 index, ServerContainer &server) {
                // AKA add/accept connection with a new session.
                u32 new_handle;
                BIO_RES_TRY(svc::AcceptSession(new_handle, server.handle.handle));

                if(this->servers.IsFull()) {
                    svc::CloseHandle(new_handle);
                    return hipc::result::ResultOutOfServerSessionMemory;
                }

                u32 fwd_handle = InvalidHandle;
                if(server.IsMitmService()) {
                    service::sm::MitmProcessInfo info;

                    service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                    BIO_RES_TRY(sm);
                    BIO_RES_TRY(service::sm::UserNamedPortSession->AtmosphereAcknowledgeMitmSession(server.service_name, info, fwd_handle));
                }
                BIO_RES_TRY(this->AddNewSession(server, new_handle, fwd_handle));

                return ResultSuccess;
            }

            i32 GetHandleIndex(u32 handle) {
                for(u32 i = 0; i < this->servers.GetSize(); i++) {
                    auto &server = this->servers.GetAt(i);
                    if(handle == server.handle.handle) {
                        return i;
                    }
                }
                return InvalidIndex;
            }

            Result ProcessSignaledHandle(u32 index) {
                auto &server = this->servers.GetAt(index);
                switch(server.handle.type) {
                    case WaitHandleType::Server: {
                        BIO_RES_TRY(this->ProcessServerHandle(index, server));
                        break;
                    }
                    case WaitHandleType::Session: {
                        BIO_RES_TRY(this->ProcessSessionHandle(index, server));
                        break;
                    }
                }

                return ResultSuccess;
            }
            
            template<typename S>
            static inline Result CreateSession(ServerContainer &out_server, u32 session_handle) {
                Server *server;
                BIO_RES_TRY(NewServer<S>(server));

                out_server.server = server;
                out_server.new_fn = &NewServer<S>;
                out_server.delete_fn = &DeleteServer<S>;
                out_server.handle = { session_handle, WaitHandleType::Session };
                out_server.forward_handle = InvalidHandle;
                out_server.is_mitm_service = false;
                out_server.service_name = service::sm::InvalidServiceName;
                return ResultSuccess;
            }

            template<typename S>
            static inline Result CreateServer(ServerContainer &out_server, u32 port_handle, service::sm::ServiceName service_name, bool is_mitm_service) {
                Server *server;
                BIO_RES_TRY(NewServer<S>(server));

                out_server.server = server;
                out_server.new_fn = &NewServer<S>;
                out_server.delete_fn = &DeleteServer<S>;
                out_server.handle = { port_handle, WaitHandleType::Server };
                out_server.forward_handle = InvalidHandle;
                out_server.is_mitm_service = is_mitm_service;
                out_server.service_name = service_name;
                return ResultSuccess;
            }

            template<typename S>
            static inline Result Create(ServerObject *&out_obj, ServerContainer server) {
                BIO_RES_TRY(mem::New(out_obj, &CallHandler<S>, server));

                return ResultSuccess;
            }

    };

    class ServerManager;

    ServerManager &GetMitmQueryManager();
    Result EnsureMitmQueryThreadLaunched();

    class ServerManager {

        private:
            util::LinkedList<ServerObject*> server_objects;
            util::SizedArray<u32, os::MaxWaitObjectCount> wait_handles;
            util::LinkedList<os::Thread> process_threads;

            template<ShouldMitmFunction ShouldMitmFn>
            Result RegisterMitmQuerySession(u32 mitm_query_handle) {
                auto &manager = GetMitmQueryManager();
                BIO_RES_TRY(manager.RegisterSession<MitmQueryServer<ShouldMitmFn>>(mitm_query_handle));
                BIO_RES_TRY(EnsureMitmQueryThreadLaunched());

                return ResultSuccess;
            }

            void PrepareWaitHandles() {
                this->wait_handles.Clear();
                for(u32 i = 0; i < this->server_objects.GetSize(); i++) {
                    auto &server_object = this->server_objects.GetAt(i);
                    auto &servers = server_object->GetServers();
                    for(u32 j = 0; j < servers.GetSize(); j++) {
                        auto &server = servers.GetAt(j);
                        if(server.handle.handle != InvalidHandle) {
                            this->wait_handles.Push(server.handle.handle);
                        }
                    }
                }
            }

        public:
            template<typename S>
            Result RegisterServerContainer(ServerContainer &server) {
                static_assert(IsServer<S>, "Must be a Server type");

                ServerObject *server_object;
                BIO_RES_TRY(ServerObject::Create<S>(server_object, server));
                BIO_RES_TRY(this->server_objects.PushBack(server_object));

                return ResultSuccess;
            }

            template<typename S>
            Result RegisterServer(u32 port_handle, service::sm::ServiceName service_name, bool is_mitm_service) {
                static_assert(IsServer<S>, "Must be a Server type");

                ServerContainer server;
                BIO_RES_TRY(ServerObject::CreateServer<S>(server, port_handle, service_name, is_mitm_service));
                BIO_RES_TRY(this->RegisterServerContainer<S>(server));

                return ResultSuccess;
            }

            template<typename S>
            Result RegisterSession(u32 session_handle) {
                static_assert(IsServer<S>, "Must be a Server type");

                ServerContainer server;
                BIO_RES_TRY(ServerObject::CreateSession<S>(server, session_handle));
                BIO_RES_TRY(this->RegisterServerContainer<S>(server));

                return ResultSuccess;
            }

            template<typename S>
            Result RegisterServiceServer() {
                static_assert(IsService<S>, "Must be a Service type");

                const auto name = service::sm::ServiceName::Encode(S::GetName());
                u32 port_handle;

                service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                BIO_RES_TRY(sm);
                BIO_RES_TRY(service::sm::UserNamedPortSession->RegisterService(name, false, S::GetMaxSessions(), port_handle));

                BIO_RES_TRY(this->RegisterServer<S>(port_handle, name, false));
                
                return ResultSuccess;
            }

            template<typename S>
            Result RegisterMitmServiceServer() {
                static_assert(IsMitmService<S>, "Must be a MitmService type");

                const auto name = service::sm::ServiceName::Encode(S::GetName());
                u32 port_handle;
                u32 mitm_query_handle;
                
                service::ScopedSessionGuard sm(service::sm::UserNamedPortSession);
                BIO_RES_TRY(sm);
                BIO_RES_TRY(service::sm::UserNamedPortSession->AtmosphereInstallMitm(name, port_handle, mitm_query_handle));

                BIO_RES_TRY(this->RegisterMitmQuerySession<&S::ShouldMitm>(mitm_query_handle));
                BIO_RES_TRY(this->RegisterServer<S>(port_handle, name, true));

                return ResultSuccess;
            }

            template<typename S>
            Result RegisterNamedPortServer() {
                static_assert(IsNamedPort<S>, "Must be a NamedPort type");

                u32 port_handle;
                BIO_RES_TRY(svc::ManageNamedPort(port_handle, S::GetPortName(), S::GetMaxSessions()));
                BIO_RES_TRY(this->RegisterServer<S>(port_handle, service::sm::InvalidServiceName, false));

                return ResultSuccess;
            }

            Result Process() {
                this->PrepareWaitHandles();

                auto out_idx = ServerObject::InvalidIndex;
                BIO_RES_TRY(os::WaitHandlesAny(this->wait_handles, svc::IndefiniteWait, out_idx));

                if((out_idx >= 0) && (out_idx < this->wait_handles.GetSize())) {
                    auto &signaled_handle = this->wait_handles.GetAt(out_idx);

                    ServerObject *server_object;
                    auto it = this->server_objects.Iterate();
                    while(it.GetNext(server_object)) {
                        auto idx = server_object->GetHandleIndex(signaled_handle);
                        if(idx != ServerObject::InvalidIndex) {
                            // The handle belongs to a server from the server object.
                            BIO_RES_TRY(server_object->ProcessSignaledHandle(idx));
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
                    // TODO: any other result we should check?
                }
            }

    };

}