
#pragma once
#include <bio/service/dispdrv/dispdrv_Types.hpp>
#include <bio/service/vi/vi_Types.hpp>

namespace bio::service::dispdrv {

    class HOSBinderDriver : public ipc::client::Service {

        public:
            using Service::Service;

        public:
            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "dispdrv";
            }

        public:
            inline Result TransactParcel(i32 binder_handle, ParcelTransactionId transaction_id, u32 flags, void *in_parcel, u64 in_parcel_size, void *out_parcel, u64 out_parcel_size) {
                return this->session.SendRequestCommand<0>(ipc::client::In<i32>(binder_handle), ipc::client::In<ParcelTransactionId>(transaction_id), ipc::client::In<u32>(flags), ipc::client::Buffer(in_parcel, in_parcel_size, ipc::BufferAttribute::In | ipc::BufferAttribute::MapAlias), ipc::client::Buffer(out_parcel, out_parcel_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias));
            }
            
            inline Result AdjustRefcount(i32 binder_handle, i32 add_value, RefcountType type) {
                return this->session.SendRequestCommand<1>(ipc::client::In<i32>(binder_handle), ipc::client::In<i32>(add_value), ipc::client::In<RefcountType>(type));
            }

            inline Result GetNativeHandle(i32 binder_handle, u32 unk, u32 &out_handle) {
                return this->session.SendRequestCommand<2>(ipc::client::In<i32>(binder_handle), ipc::client::In<u32>(unk), ipc::client::OutHandle<ipc::HandleMode::Copy>(out_handle));
            }

            inline Result TransactParcelAuto(i32 binder_handle, ParcelTransactionId transaction_id, u32 flags, void *in_parcel, u64 in_parcel_size, void *out_parcel, u64 out_parcel_size) {
                return this->session.SendRequestCommand<3>(ipc::client::In<i32>(binder_handle), ipc::client::In<ParcelTransactionId>(transaction_id), ipc::client::In<u32>(flags), ipc::client::Buffer(in_parcel, in_parcel_size, ipc::BufferAttribute::In | ipc::BufferAttribute::AutoSelect), ipc::client::Buffer(out_parcel, out_parcel_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::AutoSelect));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(HOSBinderDriver);

}