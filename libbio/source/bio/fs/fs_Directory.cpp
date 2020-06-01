#include <bio/fs/fs_Directory.hpp>

namespace bio::fs {

    Result Directory::UpdateEntries() {
        this->iter_offset = 0;
        mem::ZeroCount(this->entry_buf, this->max_entry_count);
        BIO_RES_TRY(this->dir->Read(this->entry_buf, this->max_entry_count * sizeof(service::fsp::DirectoryEntry), this->entry_buf_count));

        return ResultSuccess;
    }

    Result Directory::StartIterating(u64 max_entry_count) {
        if(this->IsIterating()) {
            this->StopIterating();
        }
        this->max_entry_count = max_entry_count;
        BIO_RES_TRY(mem::AllocateCount(this->max_entry_count, this->entry_buf));

        BIO_RES_TRY(this->UpdateEntries());
        return ResultSuccess;
    }

    bool Directory::GetNextEntry(service::fsp::DirectoryEntry &out_entry) {
        if(this->IsIterating()) {
            if((this->entry_buf_count > 0) && (this->iter_offset >= this->entry_buf_count)) {
                BIO_RET_UNLESS(this->UpdateEntries().IsSuccess(), false);
            }
            if(this->iter_offset < this->entry_buf_count) {
                auto entry_ref = &this->entry_buf[this->iter_offset];
                mem::Copy(&out_entry, entry_ref);
                this->iter_offset++;
                return true;
            }
        }
        return false;
    }

    void Directory::StopIterating() {
        if(this->IsIterating()) {
            mem::Free(this->entry_buf);
            this->entry_buf = nullptr;
            this->entry_buf_count = 0;
            this->iter_offset = 0;
            this->max_entry_count = 0;
        }
    }

    Result Directory::GetEntryCount(u64 &out_count) {
        BIO_RES_TRY(this->dir->GetEntryCount(out_count));
        return ResultSuccess;
    }

}