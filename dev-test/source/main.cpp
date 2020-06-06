
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/dyn/dyn_Module.hpp>
#include <bio/util/util_List.hpp>
#include <bio/fs/fs_Api.hpp>
#include <bio/os/os_Version.hpp>

using namespace bio;

BIO_CRT0_DEFINE_MODULE_NAME("dev-test");

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

class AccService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "acc:u0";
        }

    public:
        inline Result GetUserCount(u32 &out_count) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_count));
        }

};

class BioDevService : public ipc::client::Service {

    public:
        using Service::Service;

        static inline constexpr bool IsDomain = false;

        static inline constexpr const char *GetName() {
            return "bio-dev";
        }

    public:
        inline Result Sample0(u32 &out_v) {
            return this->session.SendRequestCommand<0>(ipc::client::Out<u32>(out_v));
        }

};

void DevMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<BioDevService> biodev;
    BIO_DIAG_RES_ASSERT(service::CreateService(biodev));

    u32 val;
    BIO_DIAG_RES_ASSERT(biodev->Sample0(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

void AccMitmMain() {
    BIO_DIAG_LOG("Main()");

    mem::SharedObject<AccService> acc;
    BIO_DIAG_RES_ASSERT(service::CreateService(acc));

    u32 val;
    BIO_DIAG_RES_ASSERT(acc->GetUserCount(val));

    BIO_DIAG_LOGF("Got value: %d", val);

    crt0::Exit(val);
}

#define FS_LOG(fmt, ...) ({ \
    mem::SharedObject<fs::File> f; \
    BIO_DIAG_RES_ASSERT(fs::OpenFile("sd:/fs_test_log.log", service::fsp::FileOpenMode::Write | service::fsp::FileOpenMode::Append, f)); \
    char buf[0x400]; \
    mem::ZeroArray(buf); \
    const auto buf_len = static_cast<u32>(util::SNPrintf(buf, sizeof(buf), fmt, ##__VA_ARGS__)); \
    BIO_DIAG_RES_ASSERT(f->Write(buf, buf_len)); \
})

void FsMain() {
    BIO_DIAG_LOG("Main()");

    BIO_DIAG_RES_ASSERT(service::fsp::FileSystemServiceSession.Initialize());
    BIO_DIAG_RES_ASSERT(fs::MountSdCard("sd"));
    
    fs::CreateFile("sd:/file", service::fsp::FileAttribute::None, 0);
    fs::DeleteFile("sd:/file");

    fs::CreateDirectory("sd:/dir");
    fs::CreateDirectory("sd:/dir/dir2");
    fs::CreateDirectory("sd:/dir/../newdir");

    service::fsp::DirectoryEntryType type;
    fs::GetEntryType("sd:/newdir", type);

    BIO_DIAG_LOGF("Type: %s", (type == service::fsp::DirectoryEntryType::Directory) ? "Directory" : "File");

    BIO_DIAG_LOG("Done");
}

void Main() {
}