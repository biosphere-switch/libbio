#include <bio/fs/fs_File.hpp>

namespace bio::fs {

    Result File::GetSize(u64 &out_size) {
        BIO_RES_TRY(this->file->GetSize(out_size));
        return ResultSuccess;
    }

    Result File::Seek(u64 pos, Whence whence) {
        switch(whence) {
            case Whence::Begin: {
                this->cur_offset = pos;
                break;
            }
            case Whence::Current: {
                this->cur_offset += pos;
                break;
            }
            case Whence::End: {
                u64 size = 0;
                BIO_RES_TRY(this->GetSize(size));
                this->cur_offset = size;
                break;
            }
        }
        return ResultSuccess;
    }

    Result File::Read(void *buf, u64 size, u64 &out_read) {
        u64 tmp_read;
        BIO_RES_TRY(this->file->Read(this->cur_offset, buf, size, service::fsp::FileReadOption::None, tmp_read));

        this->cur_offset += tmp_read;
        out_read = tmp_read;
        return ResultSuccess;
    }

    Result File::Write(void *buf, u64 size, u64 &out_written) {
        BIO_RES_TRY(this->file->Write(this->cur_offset, buf, size, service::fsp::FileWriteOption::Flush));
        
        this->cur_offset += size;
        out_written = size;
        return ResultSuccess;
    }

}