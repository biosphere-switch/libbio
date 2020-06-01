
#pragma once
#include <bio/service/service_Types.hpp>
#include <bio/service/fsp/fsp_Types.hpp>
#include <bio/service/fsp/fsp_Results.hpp>

namespace bio::service::fsp {

    class Directory : public ipc::client::SessionObject {

        public:
            using SessionObject::SessionObject;

        public:
            inline Result Read(DirectoryEntry *out_entry_buf, u64 entry_buf_size, u64 &out_read_entry_count) {
                return this->session.SendRequestCommand<0>(ipc::client::Out<u64>(out_read_entry_count), ipc::client::Buffer(out_entry_buf, entry_buf_size, ipc::BufferAttribute::Out | ipc::BufferAttribute::MapAlias));
            }

            inline Result GetEntryCount(u64 &out_count) {
                return this->session.SendRequestCommand<1>(ipc::client::Out<u64>(out_count));
            }

    };

}