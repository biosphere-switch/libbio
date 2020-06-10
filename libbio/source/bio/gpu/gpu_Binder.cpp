#include <bio/gpu/gpu_Binder.hpp>
#include <bio/gpu/gpu_Impl.hpp>

namespace bio::gpu {

    Result Binder::TransactParcel(service::dispdrv::ParcelTransactionId transaction_id, ParcelPayload &payload, u64 payload_size, Parcel &out_response_parcel) {
        mem::SharedObject<service::dispdrv::HOSBinderDriver> hos_binder_driver;
        // TODO: maybe make this non-dependent on ::gpu's HOSBinderDriver session?
        BIO_RES_TRY(GetHOSBinderDriver(hos_binder_driver));

        ParcelPayload response_parcel_payload;
        // Are transaction flags relevant here? other libraries seem to use always 0...
        BIO_RES_TRY(hos_binder_driver->TransactParcel(this->handle, transaction_id, 0, &payload, payload_size, &response_parcel_payload, sizeof(response_parcel_payload)));
        BIO_RES_TRY(out_response_parcel.LoadFrom(response_parcel_payload));
        
        return ResultSuccess;
    }

    Result Binder::DecreaseRefcounts() {
        mem::SharedObject<service::dispdrv::HOSBinderDriver> hos_binder_driver;
        // TODO: maybe make this non-dependent on ::gpu's HOSBinderDriver session?
        BIO_RES_TRY(GetHOSBinderDriver(hos_binder_driver));

        BIO_RES_TRY(hos_binder_driver->AdjustRefcount(this->handle, -1, service::dispdrv::RefcountType::Strong));
        BIO_RES_TRY(hos_binder_driver->AdjustRefcount(this->handle, -1, service::dispdrv::RefcountType::Weak));

        return ResultSuccess;
    }

}