
#pragma once
#include <bio/service/hid/hid_AppletResource.hpp>
#include <bio/service/hid/hid_Types.hpp>

namespace bio::service::hid {

    class HidService : public ipc::client::Service {

        public:
            using Service::Service;

            static inline constexpr bool IsDomain = false;

            static inline constexpr const char *GetName() {
                return "hid";
            }

        public:
            inline Result CreateAppletResource(u64 aruid, mem::SharedObject<AppletResource> &out_applet_res) {
                return this->session.SendRequestCommand<0>(ipc::client::InProcessId(aruid), ipc::client::OutSessionObject<0, AppletResource>(out_applet_res));
            }

            inline Result SetSupportedNpadStyleSet(u64 aruid, NpadStyleTag npad_style_tag) {
                return this->session.SendRequestCommand<100>(ipc::client::In(npad_style_tag), ipc::client::InProcessId(aruid));
            }

            inline Result SetSupportedNpadIdType(u64 aruid, void *controllers_buf, u64 controllers_buf_size) {
                return this->session.SendRequestCommand<102>(ipc::client::InProcessId(aruid), ipc::client::Buffer(controllers_buf, controllers_buf_size, ipc::BufferAttribute::In | ipc::BufferAttribute::Pointer));
            }

            inline Result ActivateNpad(u64 aruid) {
                return this->session.SendRequestCommand<103>(ipc::client::InProcessId(aruid));
            }

            inline Result DeactivateNpad(u64 aruid) {
                return this->session.SendRequestCommand<104>(ipc::client::InProcessId(aruid));
            }

            inline Result ActivateNpadWithRevision(u64 aruid, u32 revision) {
                return this->session.SendRequestCommand<109>(ipc::client::In(revision), ipc::client::InProcessId(aruid));
            }

            inline Result SetNpadJoyAssignmentModeSingle(u64 aruid, ControllerId controller, NpadJoyDeviceType joy_type) {
                return this->session.SendRequestCommand<123>(ipc::client::In(controller), ipc::client::InProcessId(aruid), ipc::client::In(joy_type));
            }

            inline Result SetNpadJoyAssignmentModeDual(u64 aruid, ControllerId controller) {
                return this->session.SendRequestCommand<124>(ipc::client::In(controller), ipc::client::InProcessId(aruid));
            }

    };

    BIO_SERVICE_DECLARE_GLOBAL_SESSION(HidService)

}