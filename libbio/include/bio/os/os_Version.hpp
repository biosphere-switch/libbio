
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
            if(this->major == ver.major) {
                if(this->minor == ver.minor) {
                    if(this->micro == ver.micro) {
                        return true;
                    }
                }
            }
            return !this->Equals(ver) && !this->LowerThan(ver);
        }

    };

    Version GetSystemVersion();
    void SetSystemVersion(Version ver);

}