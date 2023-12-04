/* dj_fft.h - public domain FFT library
by Jonathan Dupuy

   INTERFACING

   define DJ_ASSERT(x) to avoid using assert.h.

   QUICK NOTES

*/

#pragma once

#include <cmath>    // M_PI
#include <complex>  // std::complex
#include <vector>   // std::vector

#ifndef DJ_ASSERT
#   include <cassert>
#   define DJ_ASSERT(x) assert(x)
#endif

namespace dj {

// FFT argument: std::vector<std::complex>
template <typename T> using fft_arg = std::vector<std::complex<T>>;

// FFT direction specifier
enum class fft_dir {DIR_FWD = +1, DIR_BWD = -1};

// FFT routines
template<typename T> fft_arg<T> fft1d(const fft_arg<T> &xi, const fft_dir &dir);
template<typename T> fft_arg<T> fft2d(const fft_arg<T> &xi, const fft_dir &dir);
template<typename T> fft_arg<T> fft3d(const fft_arg<T> &xi, const fft_dir &dir);

// GPU FFT routines (float precision only)
fft_arg<float> fft1d_gpu(const fft_arg<float> &xi, const fft_dir &dir);
fft_arg<float> fft2d_gpu(const fft_arg<float> &xi, const fft_dir &dir);
fft_arg<float> fft3d_gpu(const fft_arg<float> &xi, const fft_dir &dir);

// GPU FFT routines (for advanced users: create an OpenGL context yourself)
fft_arg<float> fft1d_gpu_glready(const fft_arg<float> &xi, const fft_dir &dir);
fft_arg<float> fft2d_gpu_glready(const fft_arg<float> &xi, const fft_dir &dir);
fft_arg<float> fft3d_gpu_glready(const fft_arg<float> &xi, const fft_dir &dir);

// Raw array FFT routines
template <typename T> using fft_arg_raw = std::complex<T>*;
template<typename T> void fft1d(fft_arg_raw<T>& xi,
                                fft_arg_raw<T>& xo,
                                const fft_dir& dir,
                                const uint32_t& sz);


// ----------------------------------------------------------------

// Overloaded version of fft1d() that takes in both an input vector
// (xi) and an output vector (xo), modifying the output vector
// directly instead of allocating additional memory for a return
// vector.
template <typename T> void fft1d(const fft_arg<T>& xi,
                                 fft_arg<T>& xo,
                                 const fft_dir& dir)
{
    DJ_ASSERT((xi.size() & (xi.size() - 1)) == 0 && "invalid input size");
    int cnt = (int)xi.size();
    int msb = findMSB(cnt);
    T nrm = T(1) / std::sqrt(T(cnt));

    // pre-process the input data
    for (int j = 0; j < cnt; ++j)
        xo[j] = nrm * xi[bitr(j, msb)];

    // fft passes
    for (int i = 0; i < msb; ++i) {
        int bm = 1 << i; // butterfly mask
        int bw = 2 << i; // butterfly width
        T ang = T(dir) * M_PI / T(bm); // precomputation

        // fft butterflies
        for (int j = 0; j < (cnt / 2); ++j) {
            int i1 = ((j >> i) << (i + 1)) + j % bm; // left wing
            int i2 = i1 ^ bm;                        // right wing
            std::complex<T> z1 =
              std::polar(T(1), ang * T(i1 ^ bw)); // left wing rotation
            std::complex<T> z2 =
              std::polar(T(1), ang * T(i2 ^ bw)); // right wing rotation
            std::complex<T> tmp = xo[i1];

            xo[i1] += z1 * xo[i2];
            xo[i2] = tmp + z2 * xo[i2];
        }
    }
}

// ----------------------------------------------------------------

// Additional declarations
int findMSB(int x);
int bitr(uint32_t x, int nb);

} // namespace dj

//
//
//// end header file ///////////////////////////////////////////////////////////

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2019 Jonathan Dupuy
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

