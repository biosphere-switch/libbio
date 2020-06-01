
#pragma once
#include <bio/fs/fs_Types.hpp>

namespace bio::fs {

    class Directory {

        private:
            mem::SharedObject<service::fsp::Directory> dir;
            u64 iter_offset;
            service::fsp::DirectoryEntry *entry_buf;
            u64 entry_buf_count;
            u64 max_entry_count;

            Result UpdateEntries();

        public:
            Directory(mem::SharedObject<service::fsp::Directory> dir) : dir(dir), iter_offset(0), entry_buf(nullptr), entry_buf_count(0) {}

            ~Directory() {
                this->StopIterating();
            }

            inline constexpr bool IsIterating() {
                return this->entry_buf != nullptr;
            }

            Result StartIterating(u64 max_entry_count);

            bool GetNextEntry(service::fsp::DirectoryEntry &out_entry);

            void StopIterating();

            Result GetEntryCount(u64 &out_count);

    };

}