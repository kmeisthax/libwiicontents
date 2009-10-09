/*-------------------------------------------------------------
 
wc_private.h - wiicontents internal functions

Copyright (C) 2009 kmeisthax
Unless other credit specified
 
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.
 
Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:
 
1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.
 
2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.
 
3.This notice may not be removed or altered from any source
distribution.
 
-------------------------------------------------------------*/

#ifndef __WC_PRIVATE_C__
#define __WC_PRIVATE_C__

#include <stdlib.h>
#include <gccore.h>

//From AnyTitle Deleter
// Turn upper and lower into a full title ID
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
// Get upper or lower half of a title ID
#define TITLE_UPPER(x)		((u32)((x) >> 32))
// Turn upper and lower into a full title ID
#define TITLE_LOWER(x)		((u32)(x))

#define ISALIGNED(x) ((((u32)x)&0x1F)==0)
#define ALIGN_32    __attribute__ ((aligned (32)))

//A dirent is the size of a DOS filename plus . and \0. 8 + 1 + 3 + 1 = 13
#define ISFS_DIRENT_SIZE 13

void* memalign(unsigned int, unsigned int);
void hex2u32(const char* inhex, u32* out32);
s32 str2u16(u16* outUtf16, size_t length, const char* inAscii, size_t* copied);

#endif
