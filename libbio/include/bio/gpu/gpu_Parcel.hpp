
#pragma once
#include <bio/gpu/gpu_Results.hpp>

namespace bio::gpu {

    struct ParcelHeader {
        u32 payload_size;
        u32 payload_offset;
        u32 objects_size;
        u32 objects_offset;
    };

    struct ParcelPayload {
        ParcelHeader header;
        u8 payload[0x200];
    };

    struct ParcelData {
        u32 type;
        u32 unk;
        i32 handle;
        u8 zero_1[0xC];
        char dispdrv[8];
        u8 zero_2[0x8];
    };

    class Parcel {

        private:
            ParcelPayload payload;
            u64 read_head;
            u64 write_head;
            bool writing_finalized;

        public:
            constexpr Parcel() : read_head(0), write_head(0), writing_finalized(false) {}

            Result ReadRaw(void *out_data, u64 data_size, bool aligned = true) {
                // Sizes are almost always aligned, except for very specific reads.
                auto size = data_size;
                if(aligned) {
                    size = (data_size + 3) & ~3;
                }
                BIO_RET_UNLESS((this->read_head + size) <= sizeof(this->payload.payload), result::ResultNotEnoughReadSpace);
                
                mem::Copy(out_data, this->payload.payload + this->read_head, size);
                this->read_head += size;
                return ResultSuccess;
            }

            Result WriteRaw(void *data, u64 data_size, bool aligned = true) {
                // Sizes are almost always aligned, except for very specific writes.
                auto size = data_size;
                if(aligned) {
                    size = (data_size + 3) & ~3;
                }
                BIO_RET_UNLESS((this->write_head + size) <= sizeof(this->payload.payload), result::ResultNotEnoughWriteSpace);
                
                mem::Copy(this->payload.payload + this->write_head, data, size);
                this->write_head += size;
                return ResultSuccess;
            }

            template<typename T>
            Result Read(T &out, bool aligned = true) {
                BIO_RES_TRY(this->ReadRaw(&out, sizeof(T), aligned));
                return ResultSuccess;
            }

            template<typename T>
            Result Write(T val, bool aligned = true) {
                BIO_RES_TRY(this->WriteRaw(&val, sizeof(T), aligned));
                return ResultSuccess;
            }

            Result WriteString(const char *str) {
                // First write the string's length, then the string itself as UTF-16.
                auto len = static_cast<u32>(BIO_UTIL_STRLEN(str));
                BIO_RES_TRY(this->Write(len));
                len++;
                for(u32 i = 0; i <= len; i++) {
                    const auto chr = static_cast<u16>(str[i]);
                    // Since all the u16s are treated as a single data block, write them without aligning their size (would be 4 instead of 2).
                    BIO_RES_TRY(this->Write(chr, false));
                }
                return ResultSuccess;
            }

            Result WriteInterfaceToken(const char *token) {
                // The interface token is preceded by u32 0x100.
                BIO_RES_TRY(this->Write<u32>(0x100));
                BIO_RES_TRY(this->WriteString(token));

                return ResultSuccess;
            }

            Result ReadSizedRaw(void *out_data, u64 &out_data_size) {
                // Special types where their size is sent before the type itself (along with fd count, which we ignore).
                i32 len;
                i32 fd_count;
                BIO_RES_TRY(this->Read(len));
                BIO_RES_TRY(this->Read(fd_count));
                BIO_RET_UNLESS(fd_count == 0, result::ResultFdsNotSupported); // Can this actually happen? We never make use of fds...

                BIO_RES_TRY(this->ReadRaw(out_data, static_cast<u64>(len)));

                out_data_size = static_cast<u64>(len);
                return ResultSuccess;
            }

            template<typename T>
            Result ReadSized(T &out) {
                u64 tmp_size;
                BIO_RES_TRY(this->ReadSizedRaw(&out, tmp_size));
                BIO_RET_UNLESS(tmp_size == sizeof(T), result::ResultParcelReadSizeMismatch);

                return ResultSuccess;
            }

            Result WriteSizedRaw(void *data, u64 data_size) {
                BIO_RES_TRY(this->Write(static_cast<i32>(data_size)));
                const i32 fd_count = 0; // We don't support fds.
                BIO_RES_TRY(this->Write(fd_count));

                BIO_RES_TRY(this->WriteRaw(data, data_size));

                return ResultSuccess;
            }
            
            template<typename T>
            Result WriteSized(T val) {
                BIO_RES_TRY(this->WriteSizedRaw(&val, sizeof(T)));
                return ResultSuccess;
            }

            Result LoadFrom(ParcelPayload &payload) {
                mem::Copy(&this->payload, &payload);
                this->read_head = 0;
                this->write_head = this->payload.header.payload_size;
                this->writing_finalized = true;
                return ResultSuccess;
            }

            ParcelPayload &FinalizeWrite(u64 &out_length) {
                this->writing_finalized = true;
                this->payload.header.payload_size = this->write_head;
                this->payload.header.payload_offset = __builtin_offsetof(ParcelPayload, payload);
                // TODO: support objects? unsure if official software makes use of them (libnx supports them...)
                this->payload.header.objects_size = 0;
                const auto payload_len = this->payload.header.payload_offset + this->payload.header.payload_size;
                this->payload.header.objects_offset = payload_len;
                out_length = payload_len;
                return this->payload;
            }

    };

}