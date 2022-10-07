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

#include "platform_support.h"

#if IMAGECORE_DETECT_SSE && __SSE4_1__

#include "cpuid.h"

static unsigned int checkCPUFeatures() {
	unsigned int eax = 0;
	unsigned int ebx = 0;
	unsigned int ecx = 0;
	unsigned int edx = 0;
	unsigned int features = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	if( (edx & (1 << 25)) != 0 ) {
		features |= kCPUFeature_SSE;
	}
	if( (edx & (1 << 26)) != 0 ) {
		features |= kCPUFeature_SSE2;
	}
	if( (ecx & (1 << 0)) != 0 ) {
		features |= kCPUFeature_SSE3;
	}
	if( (ecx & (1 << 9)) != 0 ) {
		features |= kCPUFeature_SSE3_S;
	}
	if( (ecx & (1 << 19)) != 0 ) {
		features |= kCPUFeature_SSE4_1;
	}
	if( (ecx & (1 << 20)) != 0 ) {
		features |= kCPUFeature_SSE4_2;
	}
	/*
	 // AVX currently unused.
	 if( (ecx & (1 << 28)) != 0 && (ecx & (1 << 27)) != 0 && (ecx & (1 << 26)) != 0 ) {
	 __asm__ __volatile__
	 (".byte 0x0f, 0x01, 0xd0": "=a" (eax), "=d" (edx) : "c" (0) : "cc");
	 if( (eax & 6) == 6 ) {
	 features |= kCPUFeature_AVX;
	 }
	 }
	 */
	return features;
}

static unsigned int sCPUFeatures = checkCPUFeatures();

static bool haveCPUFeature(ECPUFeature feature) {
	return (sCPUFeatures & feature) != 0;
}

bool checkForCPUSupport(ECPUFeature feature)
{
	return haveCPUFeature(feature);

}
#else
bool checkForCPUSupport(ECPUFeature feature)
{
	return true;

}
#endif

#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#include <errno.h>
	#include <io.h>

	#ifndef FILE_MAP_EXECUTE
	#define FILE_MAP_EXECUTE    0x0020
	#endif /* FILE_MAP_EXECUTE */

	static int __map_mman_error(const DWORD err, const int deferr)
	{
		if (err == 0)
			return 0;
		//TODO: implement
		return err;
	}

	static DWORD __map_mmap_prot_page(const int prot)
	{
		DWORD protect = 0;
		
		if (prot == PROT_NONE)
			return protect;
			
		if ((prot & PROT_EXEC) != 0)
		{
			protect = ((prot & PROT_WRITE) != 0) ? 
						PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
		}
		else
		{
			protect = ((prot & PROT_WRITE) != 0) ?
						PAGE_READWRITE : PAGE_READONLY;
		}
		
		return protect;
	}

	static DWORD __map_mmap_prot_file(const int prot)
	{
		DWORD desiredAccess = 0;
		
		if (prot == PROT_NONE)
			return desiredAccess;
			
		if ((prot & PROT_READ) != 0)
			desiredAccess |= FILE_MAP_READ;
		if ((prot & PROT_WRITE) != 0)
			desiredAccess |= FILE_MAP_WRITE;
		if ((prot & PROT_EXEC) != 0)
			desiredAccess |= FILE_MAP_EXECUTE;
		
		return desiredAccess;
	}

	void* mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
	{
		HANDLE fm, h;
		
		void * map = MAP_FAILED;
		
	#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4293)
	#endif

		const DWORD dwFileOffsetLow = (sizeof(off_t) <= sizeof(DWORD)) ? 
						(DWORD)off : (DWORD)(off & 0xFFFFFFFFL);
		const DWORD dwFileOffsetHigh = (sizeof(off_t) <= sizeof(DWORD)) ?
						(DWORD)0 : (DWORD)((off >> 32) & 0xFFFFFFFFL);
		const DWORD protect = __map_mmap_prot_page(prot);
		const DWORD desiredAccess = __map_mmap_prot_file(prot);

		const off_t maxSize = off + (off_t)len;

		const DWORD dwMaxSizeLow = (sizeof(off_t) <= sizeof(DWORD)) ? 
						(DWORD)maxSize : (DWORD)(maxSize & 0xFFFFFFFFL);
		const DWORD dwMaxSizeHigh = (sizeof(off_t) <= sizeof(DWORD)) ?
						(DWORD)0 : (DWORD)((maxSize >> 32) & 0xFFFFFFFFL);

	#ifdef _MSC_VER
	#pragma warning(pop)
	#endif

		errno = 0;
		
		if (len == 0 
			/* Unsupported flag combinations */
			|| (flags & MAP_FIXED) != 0
			/* Usupported protection combinations */
			|| prot == PROT_EXEC)
		{
			errno = EINVAL;
			return MAP_FAILED;
		}
		
		h = ((flags & MAP_ANONYMOUS) == 0) ? 
						(HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

		if ((flags & MAP_ANONYMOUS) == 0 && h == INVALID_HANDLE_VALUE)
		{
			errno = EBADF;
			return MAP_FAILED;
		}

		fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

		if (fm == NULL)
		{
			errno = __map_mman_error(GetLastError(), EPERM);
			return MAP_FAILED;
		}
	
		map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);

		CloseHandle(fm);
	
		if (map == NULL)
		{
			errno = __map_mman_error(GetLastError(), EPERM);
			return MAP_FAILED;
		}

		return map;
	}

	int munmap(void *addr, size_t len)
	{
		if (UnmapViewOfFile(addr))
			return 0;
			
		errno =  __map_mman_error(GetLastError(), EPERM);
		
		return -1;
	}

	int mprotect(void *addr, size_t len, int prot)
	{
		DWORD newProtect = __map_mmap_prot_page(prot);
		DWORD oldProtect = 0;
		
		if (VirtualProtect(addr, len, newProtect, &oldProtect))
			return 0;
		
		errno =  __map_mman_error(GetLastError(), EPERM);
		
		return -1;
	}

	int msync(void *addr, size_t len, int flags)
	{
		if (FlushViewOfFile(addr, len))
			return 0;
		
		errno =  __map_mman_error(GetLastError(), EPERM);
		
		return -1;
	}

	int mlock(const void *addr, size_t len)
	{
		if (VirtualLock((LPVOID)addr, len))
			return 0;
			
		errno =  __map_mman_error(GetLastError(), EPERM);
		
		return -1;
	}

	int munlock(const void *addr, size_t len)
	{
		if (VirtualUnlock((LPVOID)addr, len))
			return 0;
			
		errno =  __map_mman_error(GetLastError(), EPERM);
		
		return -1;
	}
#endif