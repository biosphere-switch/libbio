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

    int Printf(const char* format, ...);

    int SPrintf(char* buffer, const char* format, ...);

    int SNPrintf(char* buffer, u64 count, const char* format, ...);

    int VSNPrintf(char* buffer, u64 count, const char* format, __builtin_va_list va);

    int VPrintf(const char* format, __builtin_va_list va);

    int FctPrintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);

}