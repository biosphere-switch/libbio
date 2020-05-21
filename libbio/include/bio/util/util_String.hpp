///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

// Modified to be used here (C++, util namespace, etc.)

#pragma once
#include <bio/base.hpp>

namespace bio::util {

    i32 Printf(const char* format, ...);

    i32 SPrintf(char* buffer, const char* format, ...);

    i32 SNPrintf(char* buffer, u64 count, const char* format, ...);

    i32 VSNPrintf(char* buffer, u64 count, const char* format, __builtin_va_list va);

    i32 VPrintf(const char* format, __builtin_va_list va);

    i32 FctPrintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);

    inline constexpr u64 Strlen(const char *str) {
        const char *s;
        for(s = str; *s; ++s);
        return s - str;
    }

    inline constexpr void Strncpy(char* dst, const char* src, u64 count) {
        u64 i = 0;
        while(i++ != count && (*dst++ = *src++));
    }

    inline constexpr i32 Strcmp(const char *s1, const char *s2) {
        while((*s1 != '\0') && (*s2 != '\0') && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return *s1 - *s2;
    }

}

#define BIO_UTIL_STRLEN(v) BIO_IS_CONSTANT(v) ? __builtin_strlen(v) : ::bio::util::Strlen(v)