/*
 * MIT License
 *
 * Copyright (c) 2017 Twitter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

enum ECPUFeature
{
	kCPUFeature_SSE     = 0x01,
	kCPUFeature_SSE2    = 0x02,
	kCPUFeature_SSE3    = 0x04,
	kCPUFeature_SSE3_S  = 0x08,
	kCPUFeature_SSE4_1  = 0x10,
	kCPUFeature_SSE4_2  = 0x20,
	kCPUFeature_AVX     = 0x40
};


bool checkForCPUSupport(ECPUFeature feature);

#if defined(_WIN32) || defined(_WIN64)
	#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
	#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
	#endif						

	/* All the headers include this file. */
	#ifndef _MSC_VER
	#include <_mingw.h>
	#endif

	#include <sys/types.h>

	#ifdef __cplusplus
	extern "C" {
	#endif
	
	#include <string.h>
	#include <ctype.h>

	#define PROT_NONE       0
	#define PROT_READ       1
	#define PROT_WRITE      2
	#define PROT_EXEC       4

	#define MAP_FILE        0
	#define MAP_SHARED      1
	#define MAP_PRIVATE     2
	#define MAP_TYPE        0xf
	#define MAP_FIXED       0x10
	#define MAP_ANONYMOUS   0x20
	#define MAP_ANON        MAP_ANONYMOUS

	#define MAP_FAILED      ((void *)-1)

	/* Flags for msync. */
	#define MS_ASYNC        1
	#define MS_SYNC         2
	#define MS_INVALIDATE   4

	void*   mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
	int     munmap(void *addr, size_t len);
	int     mprotect(void *addr, size_t len, int prot);
	int     msync(void *addr, size_t len, int flags);
	int     mlock(const void *addr, size_t len);
	int     munlock(const void *addr, size_t len);

	static int strcasecmp(const char * s1, const char * s2)
    {
        const unsigned char * u1 = (const unsigned char*) s1;
        const unsigned char * u2 = (const unsigned char*) s2;
        int result;

        while ((result = tolower(*u1) - tolower(*u2)) == 0 && *u1 != 0)
        {
            *u1++;
            *u2++;
        }

        return result;
    }

	#ifdef __cplusplus
	};
	#endif


#endif
