#include <bio/diag/diag_Assert.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/service/service_Services.hpp>

namespace bio::diag {

    __attribute__((weak))
    AssertMode g_DefaultAssertMode = AssertMode::ProcessExit;

    namespace {

        void SvcOutputAssert(const AssertMetadata &metadata) {
            svc::OutputDebugString(metadata.assertion_msg, metadata.assertion_msg_len);
        }

        void DiagLogAssert(const AssertMetadata &metadata) {
            const LogMetadata _log_metadata = {
                .source_info = metadata.source_info,
                .severity = LogSeverity::Fatal,
                .verbosity = true,
                .text_log = metadata.assertion_msg,
                .text_log_len = metadata.assertion_msg_len,
            };
            LogImpl(_log_metadata);
        }

        // __attribute__((noreturn))
        void ProcessExitAssert(const AssertMetadata &metadata) {
            crt0::Exit(static_cast<i32>(metadata.assertion_rc.GetValue()));
        }

        void FatalAssert(const AssertMetadata &metadata) {
            BIO_SERVICE_DO_WITH(sm, _sm_rc, {
                if(_sm_rc.IsSuccess()) {
                    BIO_SERVICE_DO_WITH(fatal, _fatal_rc, {
                        if(_fatal_rc.IsSuccess()) {
                            service::fatal::FatalServiceSession->ThrowWithPolicy(metadata.assertion_rc, service::fatal::Policy::ErrorScreen);
                            svc::ExitProcess();
                            __builtin_unreachable();
                        }
                    });
                }
            });
        }

        __attribute__((noreturn))
        void SvcBreakAssert(const AssertMetadata &metadata) {
            // TODO: make use of Break arguments?
            svc::Break(0, 0, 0);
            svc::ExitProcess();
            __builtin_unreachable();
        }

    }

    void AssertImpl(const AssertMetadata &metadata) {
        auto mode = metadata.assert_mode;
        if(mode == AssertMode::Default) {
            mode = g_DefaultAssertMode;
        }

        if(static_cast<bool>(mode & AssertMode::SvcOutput)) {
            SvcOutputAssert(metadata);
        }
        if(static_cast<bool>(mode & AssertMode::DiagLog)) {
            DiagLogAssert(metadata);
        }
        if(static_cast<bool>(mode & AssertMode::ProcessExit)) {
            ProcessExitAssert(metadata);
        }
        if(static_cast<bool>(mode & AssertMode::Fatal)) {
            FatalAssert(metadata);
        }
        if(static_cast<bool>(mode & AssertMode::SvcBreak)) {
            SvcBreakAssert(metadata);
        }
    }

}