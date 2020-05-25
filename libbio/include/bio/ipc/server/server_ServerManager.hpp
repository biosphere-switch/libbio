
#pragma once
#include <bio/ipc/server/server_Server.hpp>
#include <bio/ipc/server/server_Results.hpp>
#include <bio/service/sm/sm_UserNamedPort.hpp>

namespace bio::ipc::server {

    class ServerObject {

        private:
            using CallHandlerFunction = void(*)(Server*, u32, CommandContext&);
            using NewServerFunction = Result(*)(Server*&);
            using DeleteServerFunction = void(*)(Server*);

            template<typename S>
            static inline void CallHandler(Server *server, u32 request_id, CommandContext &ctx) {
                const auto handlers = ServerCommandHandlers<S>;
                for(u32 i = 0; i < ServerCommandHandleCount<S>; i++) {
                    const auto &handler = handlers[i];
                    if(handler.request_id == request_id) {
                        (reinterpret_cast<S*>(server)->*handler.fn)(ctx);
                        break;
                    }
                }
            }

            template<typename S>
            static inline Result NewServer(Server *&out_server) {
                S *server;
                BIO_RES_TRY(mem::New<S>(server));

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
            util::SizedArray<u32, 0x40> handles;

            void HandleRequestCommand(u32 request_id, CommandContext &ctx) {
                this->handler_fn(this->server, request_id, ctx);
            }

        public:
            ServerObject(Server *server, u32 handle, service::sm::ServiceName name, CallHandlerFunction handler_fn, DeleteServerFunction delete_fn) : server(server), handler_fn(handler_fn), delete_fn(delete_fn), service_name(name) {
                this->handles.Push(handle);
            }

            ~ServerObject() {
                this->delete_fn(this->server);
                this->server = nullptr;
            }

            Result ProcessSession(i32 index) {
                auto &handle = this->handles.GetAt(index);
                i32 tmp_idx;
                BIO_RES_TRY(svc::ReplyAndReceive(tmp_idx, &handle, 1, 0, svc::IndefiniteWait));

                auto should_close_session = false;
                ipc::SessionBase base(handle);

                ipc::CommandContext ctx(base);
                auto type = ipc::CommandType::Invalid;
                ReadCommandFromTls(ctx, type);

                switch(type) {
                    case ipc::CommandType::Request: {
                        u32 rq_id;
                        auto rc = ReadRequestCommandFromTls(ctx, rq_id);
                        if(rc.IsSuccess()) {
                            this->HandleRequestCommand(rq_id, ctx);
                        }
                        else {
                            WriteRequestCommandResponseOnTls(ctx, rc);
                        }
                        break;
                    }
                    case ipc::CommandType::Close: {
                        should_close_session = true;
                        WriteCommandResponseOnTls(ctx, CommandType::Close, 0);
                        break;
                    }
                    default: {
                        WriteRequestCommandResponseOnTls(ctx, 0xAAAA);
                        break;
                    }
                }

                BIO_RES_TRY_EXCEPT(svc::ReplyAndReceive(tmp_idx, &handle, 0, handle, 0), os::result::ResultTimeOut);

                if(should_close_session) {
                    svc::CloseHandle(handle);
                    this->handles.PopAt(index);
                }

                return ResultSuccess;
            }

            Result AcceptNewSession(i32 index) {
                auto &handle = this->handles.GetAt(index);
                u32 new_handle = 0;
                BIO_RES_TRY(svc::AcceptSession(new_handle, handle));

                if(this->handles.IsFull()) {
                    svc::CloseHandle(new_handle);
                    return 0xbabe;
                }

                this->handles.Push(new_handle);
                return ResultSuccess;
            }

            Result Process() {
                i32 out_idx = -1;
                BIO_RES_TRY(svc::WaitSynchronization(out_idx, this->handles.Get(), this->handles.GetSize(), svc::IndefiniteWait));
                
                BIO_RET_UNLESS(out_idx >= 0 && out_idx < this->handles.GetSize(), 0xbeef);

                if(out_idx != 0) {
                    BIO_RES_TRY(this->ProcessSession(out_idx));
                }
                else {
                    BIO_RES_TRY(this->AcceptNewSession(out_idx));
                }

                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            static inline Result Create(ServerObject *&out_obj, u32 handle, service::sm::ServiceName name, SArgs &&...s_args) {
                Server *server;
                BIO_RES_TRY(NewServer<S>(server, s_args...));

                BIO_RES_TRY(mem::New<ServerObject>(out_obj, server, handle, name, &CallHandler<S>, &DeleteServer<S>));

                return ResultSuccess;
            }

    };

    template<u32 ServerCount>
    class ServerManager {

        private:
            util::SizedArray<ServerObject*, ServerCount> servers;

        public:
            template<typename S, typename ...SArgs>
            Result RegisterServer(u32 handle, SArgs &&...s_args) {
                ServerObject *obj;
                BIO_RES_TRY(ServerObject::Create<S>(obj, handle, service::sm::InvalidServiceName, s_args...));

                this->servers.Push(obj);
                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            Result RegisterServiceServer(service::sm::ServiceName name, i32 max_sessions, SArgs &&...s_args) {
                u32 handle;
                BIO_SERVICE_DO_WITH(sm, _sm_rc, {
                    BIO_RES_TRY(_sm_rc);
                    
                    BIO_RES_TRY(service::sm::UserNamedPortSession->RegisterService(name, false, max_sessions, handle));
                });
                
                ServerObject *obj;
                BIO_RES_TRY(ServerObject::Create<S>(obj, handle, name, s_args...));

                this->servers.Push(obj);
                return ResultSuccess;
            }

            template<typename S, typename ...SArgs>
            Result RegisterNamedPortServer(const char *name, i32 max_sessions, SArgs &&...s_args) {
                u32 handle;
                BIO_RES_TRY(svc::ManageNamedPort(handle, name, max_sessions));
                BIO_RES_TRY(this->RegisterServer(handle, s_args...));

                return ResultSuccess;
            }

            Result Process() {
                for(u32 i = 0; i < this->servers.GetSize(); i++) {
                    auto &server = this->servers.GetAt(i);
                    BIO_RES_TRY(server->Process());
                }

                return ResultSuccess;
            }

            void LoopProcess() {
                while(true) {
                    auto rc = Process();
                    if(rc == os::result::ResultOperationCancelled) {
                        break;
                    }
                }
            }

    };

}