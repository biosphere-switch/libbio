#include <bio/fs/fs_Api.hpp>
#include <bio/util/util_List.hpp>

namespace bio::fs {

    namespace {

        void UnpackPath(util::LinkedList<PathName> &out_unpacked_path, const char *path) {
            out_unpacked_path.Clear();

            auto cur_name = mem::Zeroed<PathName>();
            cur_name.type = PathNameType::Invalid;
            u32 offset = 0;
            u32 name_offset = 0;

            #define _PROCESS_NAME \
                cur_name.type = PathNameType::Normal; \
                if((name_offset > 0) && (cur_name.name[name_offset - 1] == RootTerminator)) { \
                    cur_name.type = PathNameType::Root; \
                    cur_name.name[name_offset - 1] = Nul; \
                } \
                if((name_offset == 2) && (cur_name.name[0] == '.') && (cur_name.name[1] == '.')) { \
                    out_unpacked_path.PopBack(); \
                } \
                else { \
                    out_unpacked_path.PushBack(cur_name); \
                } \
                cur_name = mem::Zeroed<PathName>(); \
                cur_name.type = PathNameType::Invalid; \
                name_offset = 0;
            
            const auto path_len = BIO_UTIL_STRLEN(path);
            while(offset < path_len) {
                auto &cur = path[offset];
                if(cur == PathSeparator) {
                    _PROCESS_NAME
                }
                else {
                    cur_name.name[name_offset] = cur;
                    name_offset++;
                }
                offset++;
            }
            if(name_offset > 0) {
                // There's a last pathname to add.
                _PROCESS_NAME
            }

            #undef _PROCESS_NAME
        }

        inline bool ValidateUnpackedPath(util::LinkedList<PathName> &unpacked_path) {
            // At least a root pathname must be present.
            BIO_RET_UNLESS(unpacked_path.Any(), false);
            BIO_RET_UNLESS(unpacked_path.Front().type == PathNameType::Root, false);

            // TODO: more checks?
            return true;
        }

        void PackPath(util::LinkedList<PathName> &unpacked_path, char *out_path, bool add_root) {
            if(!ValidateUnpackedPath(unpacked_path)) {
                return;
            }

            u32 offset = 0;

            #define _WRITE_PATH(ch) ({ \
                out_path[offset] = ch; \
                offset++; \
            })

            u32 i = 0;
            if(!add_root) {
                i++;
                _WRITE_PATH(PathSeparator);
            }
            for(; i < unpacked_path.GetSize(); i++) {
                auto &name = unpacked_path.GetAt(i);
                u32 name_offset = 0;
                while(true) {
                    auto &cur = name.name[name_offset];
                    if(cur == Nul) {
                        break;
                    }
                    _WRITE_PATH(cur);
                    name_offset++;
                }
                if(name.type == PathNameType::Root) {
                    _WRITE_PATH(RootTerminator);
                }
                _WRITE_PATH(PathSeparator);
            }
            _WRITE_PATH(PathSeparator);
            offset--;
            _WRITE_PATH(Nul);
        }

        util::LinkedList<Device> g_MountedDevices;
        os::Mutex g_DevicesLock;

        bool FindDeviceByName(PathName name, Device &out_dev) {
            os::ScopedMutexLock lk(g_DevicesLock);
            for(u32 i = 0; i < g_MountedDevices.GetSize(); i++) {
                auto &device = g_MountedDevices.GetAt(i);
                if(device.root_name.Equals(name)) {
                    out_dev = device;
                    return true;
                }
            }
            return false;
        }

    }

    Result MountFileSystem(const char *name, mem::SharedObject<service::fsp::FileSystem> fs) {
        os::ScopedMutexLock lk(g_DevicesLock);
        Device fs_dev;
        fs_dev.fs = fs;
        fs_dev.root_name.SetName(name);
        fs_dev.root_name.type = PathNameType::Root;

        g_MountedDevices.PushBack(util::Move(fs_dev));
        return ResultSuccess;
    }

    Result MountSdCard(const char *name) {
        BIO_RET_UNLESS(service::fsp::FileSystemServiceSession.IsInitialized(), result::ResultFspNotInitialized);

        mem::SharedObject<service::fsp::FileSystem> sd_fs;
        BIO_RES_TRY(service::fsp::FileSystemServiceSession->OpenSdCardFileSystem(sd_fs));
        
        BIO_RES_TRY(MountFileSystem(name, sd_fs));
        return ResultSuccess;
    }

    Result CreateFile(const char *path, service::fsp::FileCreateOption option, u64 size) {
        BIO_RET_UNLESS(service::fsp::FileSystemServiceSession.IsInitialized(), result::ResultFspNotInitialized);

        util::LinkedList<PathName> unpacked_path;
        UnpackPath(unpacked_path, path);
        BIO_RET_UNLESS(ValidateUnpackedPath(unpacked_path), result::ResultInvalidPath);

        auto &root = unpacked_path.Front();
        Device dev;
        BIO_RET_UNLESS(FindDeviceByName(root, dev), result::ResultDeviceNotFound);

        char fsp_path[service::fsp::MaxPathLength];
        mem::ZeroArray(fsp_path);
        PackPath(unpacked_path, fsp_path, false);
        BIO_RES_TRY(dev.fs->CreateFile(fsp_path, service::fsp::MaxPathLength, option, size));

        return ResultSuccess;
    }

    Result OpenFile(const char *path, service::fsp::FileOpenMode mode, mem::SharedObject<File> &out_file) {
        BIO_RET_UNLESS(service::fsp::FileSystemServiceSession.IsInitialized(), result::ResultFspNotInitialized);

        util::LinkedList<PathName> unpacked_path;
        UnpackPath(unpacked_path, path);
        BIO_RET_UNLESS(ValidateUnpackedPath(unpacked_path), result::ResultInvalidPath);

        auto &root = unpacked_path.Front();
        Device dev;
        BIO_RET_UNLESS(FindDeviceByName(root, dev), result::ResultDeviceNotFound);

        char fsp_path[service::fsp::MaxPathLength];
        mem::ZeroArray(fsp_path);
        PackPath(unpacked_path, fsp_path, false);

        mem::SharedObject<service::fsp::File> fsp_file;
        auto rc = dev.fs->OpenFile(fsp_path, sizeof(fsp_path), mode, fsp_file);
        if(rc == service::fsp::result::ResultPathNotFound) {
            // Create the file if it doesn't exist, and retry.
            // Note: creating as normal (non-concatenation file) by default, maybe let the dev/user choose this?
            BIO_RES_TRY(dev.fs->CreateFile(fsp_path, sizeof(fsp_path), service::fsp::FileCreateOption::None, 0));

            // Retry again - don't call the function itself, in case this causes an infinite recursion.
            BIO_RES_TRY(dev.fs->OpenFile(fsp_path, sizeof(fsp_path), mode, fsp_file));
        }
        else {
            BIO_RES_TRY(rc);
        }

        mem::SharedObject<File> file;
        BIO_RES_TRY(mem::NewShared<File>(file, fsp_file));

        if(static_cast<bool>(mode & service::fsp::FileOpenMode::Append)) {
            // If opening as append mode, set position at the end of the file by default.
            file->Seek(0, Whence::End);
        }
        out_file = util::Move(file);
        return ResultSuccess;
    }

}