
#pragma once
#include <bio/base.hpp>

namespace bio::os {

    struct Version {
        u8 major;
        u8 minor;
        u8 micro;

        constexpr Version() : major(0), minor(0), micro(0) {}
        constexpr Version(u8 major, u8 minor, u8 micro) : major(major), minor(minor), micro(micro) {}

        inline constexpr bool LowerThan(const Version &ver) {
            if(this->major < ver.major) {
                return true;
            }
            else if(this->major == ver.major) {
                if(this->minor < ver.minor) {
                    return true;
                }
                else if(this->minor == ver.minor) {
                    if(this->micro < ver.micro) {
                        return true;
                    }
                }
            }
            return false;
        }

        inline constexpr bool Equals(const Version &ver) {
            if(this->major == ver.major) {
                if(this->minor == ver.minor) {
                    if(this->micro == ver.micro) {
                        return true;
                    }
                }
            }
            return false;
        }

        inline constexpr bool HigherThan(const Version &ver) {
            return !this->Equals(ver) && !this->LowerThan(ver);
        }

    };

    Version GetSystemVersion();

}

#define _BIO_OS_SYSTEM_VERSION_BASE(fn, major, minor, micro) ::bio::os::GetSystemVersion().fn({ major, minor, micro })
#define BIO_OS_SYSTEM_VERSION_HIGHER(major, minor, micro) _BIO_OS_SYSTEM_VERSION_BASE(HigherThan, major, minor, micro)
#define BIO_OS_SYSTEM_VERSION_LOWER(major, minor, micro) _BIO_OS_SYSTEM_VERSION_BASE(LowerThan, major, minor, micro)
#define BIO_OS_SYSTEM_VERSION_EQUALS(major, minor, micro) _BIO_OS_SYSTEM_VERSION_BASE(Equals, major, minor, micro)
#define BIO_OS_SYSTEM_VERSION_EQUAL_HIGHER(major, minor, micro) BIO_OS_SYSTEM_VERSION_EQUALS(major, minor, micro) || BIO_OS_SYSTEM_VERSION_HIGHER(major, minor, micro)
#define BIO_OS_SYSTEM_VERSION_EQUAL_LOWER(major, minor, micro) BIO_OS_SYSTEM_VERSION_EQUALS(major, minor, micro) || BIO_OS_SYSTEM_VERSION_LOWER(major, minor, micro)