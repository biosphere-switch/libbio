
#include <bio/crt0/crt0_Types.hpp>
#include <bio/diag/diag_Log.hpp>
#include <bio/diag/diag_Assert.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/fs/fs_Api.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    auto g_ModuleName = BIO_CRT0_MAKE_MODULE_NAME("fs-api-test");

    constexpr u64 HeapSize = 128_MB;

    Result InitializeHeap(void *hbl_heap_address, u64 hbl_heap_size, void *&out_heap_address, u64 &out_heap_size) {
        if(hbl_heap_address != nullptr) {
            out_heap_address = hbl_heap_address;
            out_heap_size = hbl_heap_size;
        }
        else {
            void *heap_addr;
            BIO_RES_TRY(svc::SetHeapSize(heap_addr, HeapSize));

            out_heap_address = heap_addr;
            out_heap_size = HeapSize;
        }
        return ResultSuccess;
    }

}

namespace bio::diag {

    auto g_DefaultAssertMode = AssertMode::DiagLog | AssertMode::ProcessExit;

}

#define FS_LOG(fmt, ...) ({ \
    mem::SharedObject<fs::File> f; \
    BIO_DIAG_RES_ASSERT(fs::OpenFile("sd:/fs_test_log.log", service::fsp::FileOpenMode::Write | service::fsp::FileOpenMode::Append, f)); \
    char buf[0x400] = {}; \
    const auto buf_len = static_cast<u32>(util::SNPrintf(buf, sizeof(buf), fmt, ##__VA_ARGS__)); \
    BIO_DIAG_RES_ASSERT(f->Write(buf, buf_len)); \
})

void Main() {
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
    FS_LOG("This will be %s to a log file!\nMultiline log", "saved");

    BIO_DIAG_LOG("Done");
}