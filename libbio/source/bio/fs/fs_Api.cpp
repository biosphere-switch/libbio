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

        void PackPath(util::LinkedList<PathName> &name_list, char *out_path, bool add_root) {
            if(!ValidateUnpackedPath(name_list)) {
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
            for(; i < name_list.GetSize(); i++) {
                auto &name = name_list.GetAt(i);
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

        bool FindDeviceByName(PathName name, Device &out_dev) {
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

    Result CreateFile(const char *path, u32 flags, u64 size) {
        BIO_RET_UNLESS(service::fsp::FileSystemServiceSession.IsInitialized(), result::ResultFspNotInitialized);

        util::LinkedList<PathName> name_list;
        UnpackPath(name_list, path);
        BIO_RET_UNLESS(ValidateUnpackedPath(name_list), result::ResultInvalidPath);

        auto &root = name_list.Front();
        Device dev;
        BIO_RET_UNLESS(FindDeviceByName(root, dev), result::ResultDeviceNotFound);

        char fsp_path[service::fsp::MaxPathLength];
        mem::ZeroArray(fsp_path);
        PackPath(name_list, fsp_path, false);
        BIO_RES_TRY(dev.fs->CreateFile(fsp_path, service::fsp::MaxPathLength, flags, size));

        return ResultSuccess;
    }

}