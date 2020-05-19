#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_LogTypes.hpp>
#include <bio/util/util_Misc.hpp>
#include <bio/service/sm/sm_NamedPort.hpp>
#include <bio/service/lm/lm_LogService.hpp>
#include <bio/crt0/crt0_ModuleName.hpp>

namespace bio::crt0 {

    // Use our module name to identify our logs
    extern ModuleName g_ModuleName;

}

namespace bio::diag {

    namespace {

        constexpr u32 MaxStringLength = 0x7F;

        struct LogDataChunkBase {
            LogDataChunkHeader header;

            LogDataChunkBase() : header(mem::Zeroed<LogDataChunkHeader>()) {}

            inline constexpr bool IsEmpty() {
                return this->header.length == 0;
            }

            inline constexpr bool IsValid() {
                return !this->IsEmpty();
            }

            inline constexpr u64 ComputeSize() {
                if(this->IsEmpty()) {
                    return 0;
                }
                return sizeof(this->header) + this->header.length;
            }

        };

        template<typename T>
        struct LogDataChunk : public LogDataChunkBase {
            T value;

            LogDataChunk() : LogDataChunkBase(), value(mem::Zeroed<T>()) {}

            void Initialize(LogDataChunkKey key, T val) {
                this->header.key = static_cast<u8>(key);
                this->value = val;
                this->header.length = sizeof(T);
            }

        };

        struct LogStringDataChunk : public LogDataChunkBase {
            char value[MaxStringLength + 1];

            LogStringDataChunk() : LogDataChunkBase() {
                mem::ZeroArray(this->value);
            }

            void Initialize(LogDataChunkKey key, const char *val, const u32 val_len) {
                this->header.key = static_cast<u8>(key);
                const auto len = util::Min(val_len, MaxStringLength);
                util::Strncpy(this->value, val, len + 1);
                this->header.length = util::Strlen(this->value);
            }

        };

        struct LogPacketPayload {
            LogDataChunk<bool> log_session_begin;
            LogDataChunk<bool> log_session_end;
            LogStringDataChunk text_log;
            LogDataChunk<u32> line_number;
            LogStringDataChunk file_name;
            LogStringDataChunk function_name;
            LogStringDataChunk module_name;
            LogStringDataChunk thread_name;
            LogDataChunk<u64> log_packet_drop_count;
            LogDataChunk<u64> user_system_clock;
            LogStringDataChunk process_name;

            inline constexpr u64 ComputeSize() {
                return this->log_session_begin.ComputeSize() +
                this->log_session_end.ComputeSize() +
                this->text_log.ComputeSize() +
                this->line_number.ComputeSize() +
                this->file_name.ComputeSize() +
                this->function_name.ComputeSize() +
                this->module_name.ComputeSize() +
                this->thread_name.ComputeSize() +
                this->log_packet_drop_count.ComputeSize() +
                this->user_system_clock.ComputeSize() +
                this->process_name.ComputeSize();
            }

        };

        struct LogPacket {
            LogPacketHeader header;
            LogPacketPayload payload;

            inline constexpr u64 ComputeSize() {
                return sizeof(this->header) + payload.ComputeSize();
            }

        };

        LogPacket *AllocatePackets(const u32 text_log_len, u32 &out_count) {
            u64 remaining_len = text_log_len;
            u32 packet_count = 1;
            while(remaining_len > MaxStringLength) {
                packet_count++;
                remaining_len -= MaxStringLength;
            }

            auto packets = mem::AllocateCount<LogPacket>(packet_count);
            if(packets != nullptr) {
                mem::ZeroCount(packets, packet_count);
                out_count = packet_count;
            }
            return packets;
        }

        void FreePackets(LogPacket *packets) {
            mem::Free(packets);
        }

        template<typename T>
        inline u8 *EncodePayloadBase(u8 *buf, T &t, u64 size) {
            *reinterpret_cast<T*>(buf) = t;
            return buf + size;
        }

        template<typename T>
        concept IsDataChunk = requires(T t) {
            { t.header } -> util::SameAs<LogDataChunkHeader>;
            { t.IsEmpty() } -> util::SameAs<bool>;
            { t.ComputeSize() } -> util::SameAs<u64>;
        };

        template<typename T>
        inline u8 *EncodePayload(u8 *buf, T &t) {
            if constexpr(IsDataChunk<T>) {
                if(t.IsEmpty()) {
                    return buf;
                }
                return EncodePayloadBase(buf, t, t.ComputeSize());
            }
            return EncodePayloadBase(buf, t, sizeof(T));
        }

    }

    Result LogImplBase(const LogMetadata &metadata) {
        BIO_SERVICE_DO_WITH(sm, _sm_rc, {
            RES_TRY(_sm_rc);
            BIO_SERVICE_DO_WITH(lm, _lm_rc, {
                RES_TRY(_lm_rc);
                u32 packet_count = 0;
                auto packets = AllocatePackets(metadata.text_log_len, packet_count);
                if((packets != nullptr) && (packet_count > 0)) {
                    auto head_packet = &packets[0];
                    head_packet->header.flags |= static_cast<u8>(LogPacketFlags::Head);
                    auto tail_packet = &packets[packet_count - 1];
                    tail_packet->header.flags |= static_cast<u8>(LogPacketFlags::Tail);

                    head_packet->payload.file_name.Initialize(LogDataChunkKey::FileName, metadata.source_info.file_name, metadata.source_info.file_name_len);
                    head_packet->payload.function_name.Initialize(LogDataChunkKey::FunctionName, metadata.source_info.function_name, metadata.source_info.function_name_len);
                    head_packet->payload.module_name.Initialize(LogDataChunkKey::ModuleName, crt0::g_ModuleName.name, crt0::g_ModuleName.length);

                    u32 remaining_len = metadata.text_log_len;
                    auto cur_packet = head_packet;
                    const char *text_log_buf = metadata.text_log;
                    while(remaining_len > 0) {
                        const u64 cur_len = (remaining_len > MaxStringLength) ? MaxStringLength : remaining_len;
                        cur_packet->payload.text_log.Initialize(LogDataChunkKey::TextLog, text_log_buf, cur_len);
                        cur_packet++;
                        text_log_buf += cur_len;
                        remaining_len -= cur_len;
                    }

                    mem::SharedObject<service::lm::Logger> logger;
                    RES_TRY(service::lm::LogServiceSession->OpenLogger(logger));

                    for(u32 i = 0; i < packet_count; i++) {
                        auto cur_packet = &packets[i];
                        
                        cur_packet->header.flags |= static_cast<u8>(LogPacketFlags::LittleEndian);
                        cur_packet->header.severity = static_cast<u8>(metadata.severity);
                        cur_packet->header.verbosity = static_cast<u8>(metadata.verbosity);
                        
                        cur_packet->header.payload_size = cur_packet->payload.ComputeSize();
                        const u64 log_buf_size = cur_packet->ComputeSize();

                        auto log_buf = mem::Allocate<u8>(log_buf_size);
                        if(log_buf != nullptr) {
                            // Write the packet's header.
                            auto encode_buf = EncodePayload(log_buf, cur_packet->header);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.log_session_begin);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.log_session_end);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.text_log);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.line_number);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.file_name);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.function_name);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.module_name);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.thread_name);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.log_packet_drop_count);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.user_system_clock);
                            encode_buf = EncodePayload(encode_buf, cur_packet->payload.process_name);

                            auto rc = logger->Log(log_buf, log_buf_size);
                            mem::Free(log_buf);
                            if(rc.IsFailure()) {
                                FreePackets(packets);
                                return rc;
                            }
                        }
                    }
                    FreePackets(packets);
                }
            });
        });
        return ResultSuccess;
    }

    Result LogImpl(const LogMetadata &metadata) {
        RES_TRY(LogImplBase(metadata));
        return ResultSuccess;
    }

}