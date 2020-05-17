
#pragma once
#include <bio/ipc/client/client_Types.hpp>

namespace bio::ipc::client {

    enum class HandleMode {
        Copy,
        Move
    };

    enum class BufferMode {
        Normal,
        Static,
        Smart,
    };

    enum class RequestState {
        BeforeHeaderPreparation,
        BeforeRequest,
        AfterRequest,
        AfterResponseProcess,
    };

    struct BufferInfo {
        BufferMode mode;
        u32 type;
        u32 index;
        u64 buffer_size;

        static inline constexpr BufferInfo MakeNormal(u32 type) {
            return { BufferMode::Normal, type, 0, 0 };
        }

        static inline constexpr BufferInfo MakeStatic(u32 index) {
            return { BufferMode::Static, 0, index, 0 };
        }

        static inline constexpr BufferInfo MakeSmart(u64 buf_size, u32 index) {
            return { BufferMode::Smart, 0, index, buf_size };
        }

    };

    struct Buffer {

        BufferInfo info;
        void *data;
        u64 size;

        static inline constexpr Buffer MakeNormal(void *buf, u64 size, u32 type) {
            return { BufferInfo::MakeNormal(type), buf, size };
        }

        static inline constexpr Buffer MakeStatic(void *buf, u64 size, u32 index) {
            return { BufferInfo::MakeStatic(index), buf, size };
        }

        static inline constexpr Buffer MakeSmartNormal(void *buf, u64 size, u32 index, u64 expected_size) {
            if((expected_size != 0) && (size <= expected_size)) {
                return { { BufferMode::Normal, 0, 0, 0 }, nullptr, 0 };
            }
            return { { BufferMode::Normal, 0, 0, 0 }, buf, size };
        }

        static inline constexpr Buffer MakeSmartStatic(void *buf, u64 size, u32 index, u64 expected_size) {
            if((expected_size != 0) && (size <= expected_size)) {
                return { BufferInfo::MakeStatic(index), buf, size };
            }
            return { BufferInfo::MakeStatic(index), nullptr, 0 };
        }

    };

    struct BufferCommandData {
        u32 size;
        u32 address;
        u32 packed;
    };

    struct BufferReceiveData {
        u32 address;
        u32 packed;
    };

    struct BufferSendData {
        u32 packed;
        u32 address;
    };

    struct DomainHeader {
        u8 type;
        u8 object_id_count;
        u16 size;
        u32 object_id;
        u32 pad[2];
    };

    struct DomainResponse {
        u32 object_id_count;
        u32 pad[3];
    };

    static constexpr u64 MaxHandleCount = 8;
    static constexpr u64 MaxObjectIdCount = 8;
    static constexpr u64 MaxBufferCount = 8;

    namespace impl {

        template<typename T>
        inline constexpr T *GetRawBufferByOffset(void *buf, u64 offset) {
            auto raw_buf = reinterpret_cast<u8*>(buf);
            auto raw_at_offset = raw_buf + offset;
            return reinterpret_cast<T*>(raw_at_offset);
        }

        template<typename T>
        inline constexpr T GetFromBufferByOffset(void *buf, u64 offset) {
            auto raw_t = GetRawBufferByOffset<T>(buf, offset);
            return *raw_t;
        }

        template<typename T>
        inline constexpr void SetAtBufferByOffset(void *buf, u64 offset, T t) {
            auto raw_t = GetRawBufferByOffset<T>(buf, offset);
            *raw_t = t;
        }

        class OffsetCalculator {

            private:
                void *buf;
                u64 tmp_offset;

                template<typename T>
                inline constexpr void CheckTypeAlignImpl() {
                    this->tmp_offset += (alignof(T) - 1);
                    this->tmp_offset -= (this->tmp_offset % alignof(T));
                }

                template<typename T>
                inline constexpr void AddTypeImpl() {
                    this->tmp_offset += sizeof(T);
                }

            public:
                constexpr OffsetCalculator() : buf(nullptr), tmp_offset(0) {}
                constexpr OffsetCalculator(void *buf) : buf(buf), tmp_offset(0) {}
                constexpr OffsetCalculator(void *buf, u64 offset) : buf(buf), tmp_offset(offset) {}
                constexpr OffsetCalculator(u64 offset) : buf(nullptr), tmp_offset(offset) {}

                inline constexpr void Reset() {
                    this->tmp_offset = 0;
                }

                inline constexpr u64 GetCurrentOffset() {
                    return this->tmp_offset;
                }

                template<typename T>
                inline constexpr u64 GetNextOffset() {
                    this->CheckTypeAlignImpl<T>();
                    auto offset = this->tmp_offset;
                    this->AddTypeImpl<T>();
                    return offset;
                }

                template<typename T>
                inline constexpr T GetByOffset(u64 offset) {
                    return GetFromBufferByOffset<T>(this->buf, offset);
                }

                template<typename T>
                inline constexpr void SetByOffset(u64 offset, T t) {
                    return SetAtBufferByOffset<T>(this->buf, offset, t);
                }

                template<typename T>
                inline constexpr T GetNext() {
                    auto offset = this->GetNextOffset<T>();
                    return this->GetByOffset<T>(offset);
                }

                template<typename T>
                inline constexpr void SetNext(T t) {
                    auto offset = this->GetNextOffset<T>();
                    this->SetByOffset(offset, t);
                }

                template<typename T>
                inline constexpr void IncrementOffset() {
                    this->GetNextOffset<T>();
                }

        };

    }

    struct RequestData {
        void *in_raw;
        u64 in_raw_size;
        u32 in_copy_hs[MaxHandleCount];
        u32 in_copy_hs_size;
        u32 in_move_hs[MaxHandleCount];
        u32 in_move_hs_size;
        u32 in_object_ids[MaxHandleCount];
        u32 in_object_ids_size;
        Buffer in_bufs[MaxBufferCount];
        u32 in_bufs_size;
        Buffer in_bufs_static[MaxBufferCount];
        u32 in_bufs_static_size;
        bool in_pid;

        u64 out_pid;
        u32 out_hs[MaxHandleCount];
        u32 out_hs_size;
        u32 out_object_ids[MaxObjectIdCount];
        u32 out_object_ids_size;
        void *out_raw;
        u64 out_raw_size;
        Buffer out_bufs[MaxBufferCount];
        u32 out_bufs_size;
        Buffer out_bufs_static[MaxBufferCount];
        u32 out_bufs_static_size;
        Buffer out_bufs_exch[MaxBufferCount];
        u32 out_bufs_exch_size;
        
        SessionBase session_copy;

        template<typename T>
        inline constexpr T GetFromOutRawByOffset(u64 offset) {
            return impl::GetFromBufferByOffset<T>(this->out_raw, offset);
        }

        template<HandleMode Mode>
        inline constexpr void AddInHandle(u32 handle) {
            switch(Mode) {
                case HandleMode::Copy: {
                    if(this->in_copy_hs_size < MaxHandleCount) {
                        this->in_copy_hs[this->in_copy_hs_size] = handle;
                        this->in_copy_hs_size++;
                    }
                    break;
                }
                case HandleMode::Move: {
                    if(this->in_move_hs_size < MaxHandleCount) {
                        this->in_move_hs[this->in_move_hs_size] = handle;
                        this->in_move_hs_size++;
                    }
                    break;
                }
            }
        }

        inline constexpr void AddInSession(u32 object_id) {
            if(this->in_object_ids_size < MaxObjectIdCount) {
                this->in_object_ids[this->in_object_ids_size] = object_id;
                this->in_object_ids_size++;
            }
        }

        inline constexpr void AddInBuffer(Buffer buffer) {
            if(this->in_bufs_size < MaxBufferCount) {
                this->in_bufs[this->in_bufs_size] = buffer;
                this->in_bufs_size++;
            }
        }

        inline constexpr void AddInStaticBuffer(Buffer buffer) {
            if(this->in_bufs_static_size < MaxBufferCount) {
                this->in_bufs_static[this->in_bufs_static_size] = buffer;
                this->in_bufs_static_size++;
            }
        }

        inline constexpr void AddOutBuffer(Buffer buffer) {
            if(this->out_bufs_size < MaxBufferCount) {
                this->out_bufs[this->out_bufs_size] = buffer;
                this->out_bufs_size++;
            }
        }

        inline constexpr void AddOutStaticBuffer(Buffer buffer) {
            if(this->out_bufs_static_size < MaxBufferCount) {
                this->out_bufs_static[this->out_bufs_static_size] = buffer;
                this->out_bufs_static_size++;
            }
        }

        template<u32 Index>
        inline constexpr bool GetOutHandle(u32 &out_handle) {
            if(Index < this->out_hs_size) {
                out_handle = this->out_hs[Index];
                return true;
            }
            return false;
        }

        template<u32 Index>
        inline constexpr bool GetOutObjectId(u32 &out_object_id) {
            if(Index < this->out_object_ids_size) {
                out_object_id = this->out_object_ids[Index];
                return true;
            }
            return false;
        }

    };

    struct RequestArgument {
        // Derived arguments must have a "void Process(RequestData &data, RequestState state)" call
    };

    static constexpr u32 SFCI = 0x49434653;
    static constexpr u32 SFCO = 0x4F434653;

    namespace impl {

        inline void PrepareCommandHeader(RequestData &data) {
            u64 orawsz = 0;
            auto tls = os::GetThreadLocalStorage<u32>();
            if(data.session_copy.IsDomain()) {
                orawsz = data.in_raw_size;
                data.in_raw_size += sizeof(DomainHeader) + (data.in_object_ids_size * sizeof(u32));
            }
            *tls++ = (4 | (data.in_bufs_static_size << 16) | (data.in_bufs_size << 20) | (data.out_bufs_size << 24) | (data.out_bufs_exch_size << 28));
            auto fillsz = tls;
            if(data.out_bufs_static_size > 0) {
                *tls = ((data.out_bufs_static_size + 2) << 10);
            }
            else *tls = 0;
            if(data.in_pid || (data.in_copy_hs_size > 0) || (data.in_move_hs_size > 0)) {
                *tls++ |= 0x80000000;
                *tls++ = ((!!data.in_pid) | (data.in_copy_hs_size << 1) | (data.in_move_hs_size << 5));
                if(data.in_pid) {
                    tls += 2;
                }
                if(data.in_copy_hs_size > 0) {
                    for(auto i = 0; i < data.in_copy_hs_size; i++) {
                        *tls++ = data.in_copy_hs[i];
                    }
                }
                if(data.in_move_hs_size > 0) {
                    for(auto i = 0; i < data.in_move_hs_size; i++) {
                        *tls++ = data.in_move_hs[i];
                    }
                }
            }
            else {
                tls++;
            }
            if(data.in_bufs_static_size > 0) {
                for(auto i = 0; i < data.in_bufs_static_size; i++, tls += 2) {
                    auto ins = data.in_bufs_static[i];
                    auto bsd = reinterpret_cast<BufferSendData*>(tls);
                    auto uptr = reinterpret_cast<u64>(ins.data);
                    bsd->address = uptr;
                    bsd->packed = (ins.info.index | (ins.size << 16) | (((uptr >> 32) & 15) << 12) | (((uptr >> 36) & 15) << 6));
                }
            }
            if(data.in_bufs_size > 0) {
                for(auto i = 0; i < data.in_bufs_size; i++, tls += 3) {
                    auto in = data.in_bufs[i];
                    auto bcd = reinterpret_cast<BufferCommandData*>(tls);
                    bcd->size = in.size;
                    auto uptr = reinterpret_cast<u64>(in.data);
                    bcd->address = uptr;
                    bcd->packed = (in.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
            }
            if(data.out_bufs_size > 0) {
                for(u32 i = 0; i < data.out_bufs_size; i++, tls += 3) {
                    Buffer out = data.out_bufs[i];
                    auto bcd = reinterpret_cast<BufferCommandData*>(tls);
                    bcd->size = out.size;
                    auto uptr = reinterpret_cast<u64>(out.data);
                    bcd->address = uptr;
                    bcd->packed = (out.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
            }
            if(data.out_bufs_exch_size > 0) {
                for(u32 i = 0; i < data.out_bufs_exch_size; i++, tls += 3) {
                    Buffer ex = data.out_bufs_exch[i];
                    BufferCommandData *bcd = (BufferCommandData*)tls;
                    bcd->size = ex.size;
                    auto uptr = reinterpret_cast<u64>(ex.data);
                    bcd->address = uptr;
                    bcd->packed = (ex.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
            }
            u32 pad = static_cast<u32>(((16 - ((reinterpret_cast<u64>(tls)) & 15)) & 15) / 4);
            u32 *raw = (u32*)(tls + pad);
            u64 rawsz = (data.in_raw_size / 4) + 4;
            tls += rawsz;
            u16 *tls16 = (u16*)tls;
            if(data.out_bufs_static_size > 0) {
                for(u32 i = 0; i < data.out_bufs_static_size; i++) {
                    Buffer outs = data.out_bufs_static[i];
                    u64 outssz = (u64)outs.size;
                    tls16[i] = ((outssz > 0xffff) ? 0 : outssz);
                }
            }
            u64 u16s = (((2 * data.out_bufs_static_size) + 3) / 4);
            tls += u16s;
            rawsz += u16s;
            *fillsz |= rawsz;
            if(data.out_bufs_static_size > 0) {
                for(u32 i = 0; i < data.out_bufs_static_size; i++, tls += 2) {
                    Buffer outs = data.out_bufs_static[i];
                    BufferReceiveData *brd = (BufferReceiveData*)tls;
                    u64 uptr = (u64)outs.data;
                    brd->address = uptr;
                    brd->packed = ((uptr >> 32) | (outs.size << 16));
                }
            }
            void *vraw = (void*)raw;
            if(data.session_copy.IsDomain()) {
                DomainHeader *dh = (DomainHeader*)vraw;
                u32 *ooids = (u32*)(((u64)vraw) + sizeof(DomainHeader) + orawsz);
                dh->type = 1;
                dh->object_id_count = (u8)data.in_object_ids_size;
                dh->size = orawsz;
                dh->object_id = data.session_copy.GetObjectId();
                dh->pad[0] = dh->pad[1] = 0;
                if(data.in_object_ids_size > 0) {
                    for(u32 i = 0; i < data.in_object_ids_size; i++) {
                        ooids[i] = data.in_object_ids[i];
                    }
                }
                vraw = (void*)(((u64)vraw) + sizeof(DomainHeader));
            }
            data.in_raw = vraw;
        }

        inline void ProcessResponse(RequestData &data) {
            auto tls = os::GetThreadLocalStorage<u32>();
            u32 ctrl0 = *tls++;
            u32 ctrl1 = *tls++;
            if(ctrl1 & 0x80000000) {
                u32 ctrl2 = *tls++;
                if(ctrl2 & 1) {
                    u64 pid = *tls++;
                    pid |= (((u64)(*tls++)) << 32);
                    data.out_pid = pid;
                }
                u64 ohcopy = (ctrl2 >> 1) & 15;
                u64 ohmove = (ctrl2 >> 5) & 15;
                u64 oh = ohcopy + ohmove;
                u32 *aftoh = tls + oh;
                if(oh > 8) {
                    oh = 8;
                }
                if(oh > 0)
                {
                    for(u64 i = 0; i < oh; i++)
                    {
                        u32 hdl = *(tls + i);
                        data.out_hs[data.out_hs_size] = hdl;
                        data.out_hs_size++;
                    }
                }
                tls = aftoh;
            }
            u64 nst = (ctrl0 >> 16) & 15;
            u32 *aftst = tls + (nst * 2);
            if(nst > 8) {
                nst = 8;
            }
            if(nst > 0) {
                for(u32 i = 0; i < nst; i++, tls += 2) {
                    BufferSendData *bsd = (BufferSendData*)tls;
                    (void)(bsd);
                }
            }
            tls = aftst;
            u64 bsend = (ctrl0 >> 20) & 15;
            u64 brecv = (ctrl0 >> 24) & 15;
            u64 bexch = (ctrl0 >> 28) & 15;
            u64 bnum = bsend + brecv + bexch;
            void *ovraw = (void*)(((u64)(tls + (bnum * 3)) + 15) &~ 15);
            if(bnum > 8) {
                bnum = 8;
            }
            if(bnum > 0) {
                for(u32 i = 0; i < bnum; i++, tls += 3) {
                    BufferCommandData *bcd = (BufferCommandData*)tls;
                    (void)(bcd);
                }
            }
            if(data.session_copy.IsDomain()) {
                DomainResponse *dr = (DomainResponse*)ovraw;
                u32 *ooids = (u32*)(((u64)ovraw) + sizeof(DomainResponse) + data.out_raw_size);
                u32 ooidcount = dr->object_id_count;
                if(ooidcount > 8) {
                    ooidcount = 8;
                }
                if(ooidcount > 0) {
                    for(u32 i = 0; i < ooidcount; i++) {
                        data.out_object_ids[data.out_object_ids_size] = ooids[i];
                        data.out_object_ids_size++;
                    }
                }
                ovraw = (void*)(((u64)ovraw) + sizeof(DomainResponse));
            }
            data.out_raw = ovraw;
        }

        inline void CloseNonDomainObject(u32 handle) {
            auto tls = os::GetThreadLocalStorage<u32>();
            tls[0] = 2;
            tls[1] = 0;
            svc::SendSyncRequest(handle);
        }
        
        inline void CloseDomainObject(u32 handle, u32 object_id) {
            RequestData rq;
            rq.in_raw_size = sizeof(DomainHeader);
            auto tls = os::GetThreadLocalStorage<u32>();
            *tls++ = 4;
            u32 *fillsz = tls;
            *tls = 0;
            tls++;
            u32 pad = (((16 - (((u64)tls) & 15)) & 15) / 4);
            u32 *raw = (u32*)(tls + pad);
            u64 rawsz = ((rq.in_raw_size / 4) + 4);
            tls += rawsz;
            u16 *tls16 = (u16*)tls;
            u64 u16s = ((2 * rq.out_bufs_static_size + 3) / 4);
            tls += u16s;
            rawsz += u16s;
            *fillsz |= rawsz;
            DomainHeader *iraw = (DomainHeader*)raw;
            iraw->type = 2;
            iraw->object_id_count = 0;
            iraw->size = 0;
            iraw->object_id = object_id;
            iraw->pad[0] = iraw->pad[1] = 0;
            svc::SendSyncRequest(handle);
        }

        inline Result ConvertObjectToDomain(u32 handle, u32 &out_obj_id) {
            auto tls = os::GetThreadLocalStorage<u32>();
            tls[0] = 5;
            tls[1] = 8;
            tls[2] = 0;
            tls[3] = 0;
            tls[4] = SFCI;
            tls[5] = 0;
            tls[6] = 0;
            tls[7] = 0;

            auto rc = svc::SendSyncRequest(handle);
            if(rc.IsSuccess()) {
                RequestData rq = {};

                impl::OffsetCalculator off;
                off.IncrementOffset<u32>(); // u32 magic (SFCO)
                off.IncrementOffset<u32>(); // u32 version
                auto offset_rc = off.GetNextOffset<u32>();
                off.IncrementOffset<u32>(); // u32 token
                auto offset_objid = off.GetNextOffset<u32>();
                rq.out_raw_size = off.GetCurrentOffset();
                ProcessResponse(rq);

                off = impl::OffsetCalculator(rq.out_raw);
                rc = off.GetByOffset<u32>(offset_rc);
                if(rc.IsSuccess()) {
                    out_obj_id = off.GetByOffset<u32>(offset_objid);
                }
            }
            return rc;
        }

        inline Result QueryPointerBufferSize(u32 handle, u16 &out_size) {
            auto tls = os::GetThreadLocalStorage<u32>();
            tls[0] = 5;
            tls[1] = 8;
            tls[2] = 0;
            tls[3] = 0;
            tls[4] = SFCI;
            tls[5] = 0;
            tls[6] = 3;
            tls[7] = 0;

            auto rc = svc::SendSyncRequest(handle);
            if(rc.IsSuccess()) {
                auto rq = mem::Zeroed<RequestData>();

                impl::OffsetCalculator off;
                off.IncrementOffset<u32>(); // u32 magic (SFCO)
                off.IncrementOffset<u32>(); // u32 version
                auto offset_rc = off.GetNextOffset<u32>();
                off.IncrementOffset<u32>(); // u32 token
                auto offset_size = off.GetNextOffset<u32>();
                rq.out_raw_size = off.GetCurrentOffset();
                ProcessResponse(rq);

                off = impl::OffsetCalculator(rq.out_raw);
                rc = off.GetByOffset<u32>(offset_rc);
                if(rc.IsSuccess()) {
                    out_size = static_cast<u16>(off.GetByOffset<u32>(offset_size) & 0xFFFF);
                }
            }
            return rc;
        }

    }

}