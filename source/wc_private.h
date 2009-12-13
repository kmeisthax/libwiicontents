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

//The SD initialization vector for decrypting SD contents.
//For some reason, ES knows the key but not the IV.
//I'm putting the SDKey here anyway, in case we want to port this over to mini.
//Also the SD MD5 blanker as well.
u8* sd_key = {0xab, 0x01, 0xb9, 0xd8, 0xe1, 0x62, 0x2b, 0x08,
              0xaf, 0xba, 0xd8, 0x4d, 0xbf, 0xc2, 0xa5, 0x5d} ALIGN_32;
u8* sd_iv = {0x21, 0x67, 0x12, 0xe6, 0xaa, 0x1f, 0x68, 0x9f,
             0x95, 0xc5, 0xa2, 0x23, 0x24, 0xdc, 0x6a, 0x98} ALIGN_32;
u8* sd_md5 = {0x0e, 0x65, 0x37, 0x81, 0x99, 0xbe, 0x45, 0x17,
              0xab, 0x06, 0xec, 0x22, 0x45, 0x1a, 0x57, 0x93} ALIGN_32;

void* memalign(unsigned int, unsigned int);
void hex2u32(const char* inhex, u32* out32);
s32 str2u16(u16* outUtf16, size_t length, const char* inAscii, size_t* copied);

#endif
