
#pragma once
#include <bio/gpu/gpu_TransactionTypes.hpp>
#include <bio/gpu/gpu_Parcel.hpp>
#include <bio/service/dispdrv/dispdrv_HOSBinderDriver.hpp>

namespace bio::gpu {

    class Binder {

        public:
            static constexpr const char InterfaceToken[] = "android.gui.IGraphicBufferProducer";

        private:
            i32 handle;

            inline Result BeginTransactParcel(Parcel &parcel) {
                // All parcels start with the interface token.
                BIO_RES_TRY(parcel.WriteInterfaceToken(InterfaceToken));
                return ResultSuccess;
            }

            Result TransactParcel(service::dispdrv::ParcelTransactionId transaction_id, ParcelPayload &payload, u64 payload_size, Parcel &out_response_parcel);

            template<service::dispdrv::ParcelTransactionId Id>
            inline Result DoTransactParcel(Parcel &parcel, Parcel &response_parcel) {
                u64 payload_size;
                auto &payload = parcel.FinalizeWrite(payload_size);
                BIO_RES_TRY(this->TransactParcel(Id, payload, payload_size, response_parcel));
                return ResultSuccess;
            }
            
            inline Result EndTransactParcel(Parcel &parcel) {
                // Most parcels finalize with an error code - read it and adapt it to our own results if failure.
                BinderErrorCode err;
                BIO_RES_TRY(parcel.Read(err));

                BIO_RET_UNLESS(err == BinderErrorCode::Success, result::ConvertBinderErrorCode(err));
                return ResultSuccess;
            }

        public:
            constexpr Binder(i32 handle) : handle(handle) {}

            i32 GetHandle() {
                return this->handle;
            }

            Result Connect(ConnectionApi api, bool producer_controlled_by_app, QueueBufferOutput &out_qbo) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                // Unused, thus null (official software doesn't make use of this)
                const u32 producer_listener = 0;
                BIO_RES_TRY(parcel.Write(producer_listener));
                BIO_RES_TRY(parcel.Write(api));
                BIO_RES_TRY(parcel.Write(static_cast<u32>(producer_controlled_by_app)));
                
                Parcel response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::Connect>(parcel, response_parcel));
                BIO_RES_TRY(response_parcel.Read(out_qbo));

                BIO_RES_TRY(this->EndTransactParcel(response_parcel));
                return ResultSuccess;
            }

            Result Disconnect(ConnectionApi api, DisconnectMode mode) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                BIO_RES_TRY(parcel.Write(api));
                BIO_RES_TRY(parcel.Write(mode));
                
                Parcel response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::Disconnect>(parcel, response_parcel));

                BIO_RES_TRY(this->EndTransactParcel(response_parcel));
                return ResultSuccess;
            }

            Result SetPreallocatedBuffer(i32 slot, GraphicBuffer &buf) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                BIO_RES_TRY(parcel.Write(slot));
                // We always call this command with a graphic buffer...
                const auto has_input = true;
                BIO_RES_TRY(parcel.Write(static_cast<u32>(has_input)));
                
                if(has_input) {
                    BIO_RES_TRY(parcel.WriteSized(buf));
                }

                Parcel _response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::SetPreallocatedBuffer>(parcel, _response_parcel));

                return ResultSuccess;
            }

            Result RequestBuffer(i32 slot, bool &out_buffer_valid, GraphicBuffer &out_buffer) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                BIO_RES_TRY(parcel.Write(slot));
                
                Parcel response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::RequestBuffer>(parcel, response_parcel));
                u32 non_null;
                BIO_RES_TRY(response_parcel.Read(non_null));
                out_buffer_valid = static_cast<bool>(non_null);
                if(out_buffer_valid) {
                    // This is often true, but we don't actually make use of the buffer - we read it anyway.
                    BIO_RES_TRY(response_parcel.ReadSized(out_buffer));
                }

                BIO_RES_TRY(this->EndTransactParcel(response_parcel));
                return ResultSuccess;
            }

            Result DequeueBuffer(bool is_async, u32 width, u32 height, bool get_frame_timestamps, GraphicsAllocatorUsage usage, i32 &out_slot, bool &out_has_fence, MultiFence &out_fence) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                BIO_RES_TRY(parcel.Write(static_cast<u32>(is_async)));
                BIO_RES_TRY(parcel.Write(width));
                BIO_RES_TRY(parcel.Write(height));
                BIO_RES_TRY(parcel.Write(static_cast<u32>(get_frame_timestamps)));
                BIO_RES_TRY(parcel.Write(usage));

                Parcel response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::DequeueBuffer>(parcel, response_parcel));
                BIO_RES_TRY(response_parcel.Read(out_slot));
                u32 has_fence;
                BIO_RES_TRY(response_parcel.Read(has_fence));
                
                out_has_fence = static_cast<bool>(has_fence);
                if(out_has_fence) {
                    BIO_RES_TRY(response_parcel.ReadSized(out_fence));
                }

                BIO_RES_TRY(this->EndTransactParcel(response_parcel));
                return ResultSuccess;
            }

            Result QueueBuffer(i32 slot, QueueBufferInput qbi, QueueBufferOutput &out_qbo) {
                Parcel parcel;
                BIO_RES_TRY(this->BeginTransactParcel(parcel));

                BIO_RES_TRY(parcel.Write(slot));
                BIO_RES_TRY(parcel.WriteSized(qbi));

                Parcel response_parcel;
                BIO_RES_TRY(this->DoTransactParcel<service::dispdrv::ParcelTransactionId::QueueBuffer>(parcel, response_parcel));
                BIO_RES_TRY(response_parcel.Read(out_qbo));

                BIO_RES_TRY(this->EndTransactParcel(response_parcel));
                return ResultSuccess;
            }

            Result DecreaseRefcounts();

    };

}