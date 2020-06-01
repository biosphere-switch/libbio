
#pragma once
#include <bio/fs/fs_Types.hpp>

namespace bio::fs {

    enum class Whence {
        Begin,
        Current,
        End,
    };

    class File {

        private:
            mem::SharedObject<service::fsp::File> file;
            u64 cur_offset;

        public:
            File(mem::SharedObject<service::fsp::File> file) : file(file), cur_offset(0) {}

            Result GetSize(u64 &out_size);

            Result Seek(u64 pos, Whence whence);

            Result Read(void *buf, u64 size, u64 &out_read);

            inline Result Read(void *buf, u64 size) {
                u64 tmp_read;
                BIO_RES_TRY(this->Read(buf, size, tmp_read));
                return ResultSuccess;
            }

            Result Write(void *buf, u64 size, u64 &out_written);

            inline Result Write(void *buf, u64 size) {
                u64 tmp_written;
                BIO_RES_TRY(this->Write(buf, size, tmp_written));
                return ResultSuccess;
            }

    };

}