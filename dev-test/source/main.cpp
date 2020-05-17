
#include <bio/crt0/crt0_ModuleName.hpp>
#include <bio/util/util_String.hpp>
#include <bio/service/service_Services.hpp>
#include <bio/mem/mem_Memory.hpp>
#include <bio/crt0/crt0_Exit.hpp>

using namespace bio;

namespace bio::crt0 {

    __attribute__((section(".module_name")))
    volatile auto g_ModuleName = CRT0_MAKE_MODULE_NAME("my-own-module-name");

}

void ResultAssert(Result rc) {
    if(rc.IsFailure()) {
        crt0::Exit(rc.GetValue());
    }
}

#define RES_ASSERT(expr) ({ \
    const auto _tmp_rc = (expr); \
    if(_tmp_rc.IsFailure()) { \
        crt0::Exit(_tmp_rc.GetValue()); \
    } \
})

void Main() {

    RES_ASSERT(service::sm::Initialize());
    {
        mem::SharedObject<service::fsp::FileSystemService> fsp;
        RES_ASSERT(service::CreateService<service::fsp::FileSystemService>(fsp));
        {
            mem::SharedObject<service::fsp::FileSystem> sdfs;
            RES_ASSERT(fsp->OpenSdCardFileSystem(sdfs));
            {
                char path[0x301];
                mem::ZeroArray(path);
                util::SPrintf(path, "/sample-file.txt");

                sdfs->CreateFile(path, sizeof(path), 0, 0);
                {
                    mem::SharedObject<service::fsp::File> file;
                    RES_ASSERT(sdfs->OpenFile(path, sizeof(path), (1 << 1) | (1 << 2), file));

                    u64 size = 0;
                    RES_ASSERT(file->GetSize(size));

                    char buf[0x400];
                    mem::ZeroArray(buf);
                    auto len = util::SPrintf(buf, "Hello from no-std Biosphere!\nBruh moment\nHey hey");

                    RES_ASSERT(file->Write(size, buf, len, (1 << 0)));
                }
            }
        }
    }

}