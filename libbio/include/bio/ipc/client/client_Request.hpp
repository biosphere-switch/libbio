
#pragma once
#include <bio/ipc/client/client_Session.hpp>

namespace bio::ipc::client {

    template<typename T>
    struct In : public RequestArgument {

        T value;
        u64 offset;

        constexpr In(T type) : value(type), offset(0) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    impl::OffsetCalculator off(rq.in_raw_size);
                    this->offset = off.GetNextOffset<T>();
                    rq.in_raw_size = off.GetCurrentOffset();
                    break;
                }
                case RequestState::BeforeRequest: {
                    impl::OffsetCalculator off(rq.in_raw);
                    off.SetByOffset<T>(this->offset, this->value);
                    break;
                }
                default:
                    break;
            }
        }
 
    };

    struct InProcessId : public RequestArgument {

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.in_pid = true;
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<HandleMode Mode>
    struct InHandle : public RequestArgument {

        u32 handle;

        constexpr InHandle(u32 handle) : handle(handle) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddInHandle<Mode>(this->handle);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct InSession : public RequestArgument {

        Session session;

        constexpr InSession(Session session) : session(session) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state)
            {
                case RequestState::BeforeHeaderPreparation: {
                    if(this->session.IsValid()) {
                        if(this->session.IsDomain()) {
                            rq.AddInSession(this->session.GetObjectId());
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct InBuffer : public RequestArgument {
    
        Buffer buffer;

        constexpr InBuffer(const void *buf, u64 size, u32 type) : buffer(Buffer::MakeNormal(const_cast<void*>(buf), size, type)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddInBuffer(this->buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct InStaticBuffer : public RequestArgument {
    
        Buffer buffer;

        constexpr InStaticBuffer(const void *buf, u64 size, u32 index) : buffer(Buffer::MakeStatic(const_cast<void*>(buf), size, index)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddInStaticBuffer(this->buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct InSmartBuffer : public RequestArgument {
    
        Buffer normal_buffer;
        Buffer static_buffer;

        constexpr InSmartBuffer(const void *buf, u64 size, u32 index, u64 expected_size) : normal_buffer(Buffer::MakeSmartNormal(const_cast<void*>(buf), size, index, expected_size)), static_buffer(Buffer::MakeSmartStatic(const_cast<void*>(buf), size, index, expected_size)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddInBuffer(this->normal_buffer);
                    rq.AddInStaticBuffer(this->static_buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<typename T>
    struct Out : public RequestArgument {

        T &value;
        u64 offset;

        constexpr Out(T &ref) : value(ref), offset(0) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::AfterRequest: {
                    impl::OffsetCalculator off(rq.out_raw_size);
                    this->offset = off.GetNextOffset<T>();
                    rq.out_raw_size = off.GetCurrentOffset();
                    break;
                }
                case RequestState::AfterResponseProcess: {
                    impl::OffsetCalculator off(rq.out_raw);
                    this->value = off.GetByOffset<T>(this->offset);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct OutProcessId : public RequestArgument {

        u64 &pid;

        constexpr OutProcessId(u64 &pid_ref) : pid(pid_ref) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::AfterResponseProcess: {
                    this->pid = rq.out_pid;
                    break;
                }
                default:
                    break;
            }
        }

    };

    template<u32 Index>
    struct OutHandle : public RequestArgument {

        u32 &handle;

        constexpr OutHandle(u32 &handle_ref) : handle(handle_ref) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::AfterResponseProcess: {
                    rq.GetOutHandle<Index>(this->handle);
                    break;
                }
                default:
                    break;
            }
        }

    };

    /*
    template<u32 OIndex, bool AutoClear>
    class OutEvent : public RequestArgument
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

    template<u32 Index>
    struct OutSession : public RequestArgument {

        Session &session;

        constexpr OutSession(Session &session_ref) : session(session_ref) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::AfterResponseProcess: {
                    if(rq.session_copy.IsDomain()) {
                        u32 obj_id = InvalidObjectId;
                        if(rq.GetOutObjectId<Index>(obj_id)) {

                            session = Session::CreateDomainFromParent(rq.session_copy, obj_id);
                        }
                    }
                    else {
                        u32 handle = InvalidHandle;
                        if(rq.GetOutHandle<Index>(handle)) {
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

    /*
    template<u32 Index, typename S>
    struct OutSessionObject : public RequestArgument {
        static_assert(std::is_base_of_v<SessionObject, S>, "Invalid session object");

        SharedPointer<S> &session_obj;

        constexpr OutSessionObject(SharedPointer<S> &session_obj_ref) : session_obj(session_obj_ref) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::AfterResponseProcess: {
                    if(rq.session_copy.IsDomain()) {
                        u32 obj_id = InvalidObjectId;
                        if(rq.GetOutObjectId<Index>(obj_id)) {
                            auto session = Session::CreateDomainFromParent(rq.session_copy, obj_id);
                            session_obj = std::move(std::make_shared<S>(session));
                        }
                    }
                    else {
                        u32 handle = InvalidHandle;
                        if(rq.GetOutHandle<Index>(handle)) {
                            auto session = Session::CreateFromHandle(handle);
                            session_obj = std::move(std::make_shared<S>(session));
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

    };
    */

    struct OutBuffer : public RequestArgument {
    
        Buffer buffer;

        constexpr OutBuffer(void *buf, u64 size, u32 type) : buffer(Buffer::MakeNormal(buf, size, type)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddOutBuffer(this->buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct OutStaticBuffer : public RequestArgument {
    
        Buffer buffer;

        constexpr OutStaticBuffer(void *buf, u64 size, u32 index) : buffer(Buffer::MakeStatic(buf, size, index)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddOutStaticBuffer(this->buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

    struct OutSmartBuffer : public RequestArgument {
    
        Buffer normal_buffer;
        Buffer static_buffer;

        constexpr OutSmartBuffer(void *buf, u64 size, u32 index, u64 expected_size) : normal_buffer(Buffer::MakeSmartNormal(buf, size, index, expected_size)), static_buffer(Buffer::MakeSmartStatic(buf, size, index, expected_size)) {}

        inline constexpr void Process(RequestData &rq, RequestState state) {
            switch(state) {
                case RequestState::BeforeHeaderPreparation: {
                    rq.AddOutBuffer(this->normal_buffer);
                    rq.AddOutStaticBuffer(this->static_buffer);
                    break;
                }
                default:
                    break;
            }
        }

    };

}