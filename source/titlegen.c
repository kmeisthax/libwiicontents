/*-------------------------------------------------------------
 
titlegen.c - Create or find names for titles.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <ogc/isfs.h>
#include <ogc/ios.h>

#include "wiicontents.h"

//Structs
typedef struct {
    u8 buildtag[0x30]; //Some weird build tag at the beginning of a 00000000.app IMET
    u8 buildaddr[0x10]; //Another weird buildtag in an IMET.
    u8 zeroes[0x40]; // padding, 0x80 for Opening.bnr
    u32 imet; // "IMET"
    u8 unk[8];  // 0x0000060000000003 fixed, unknown purpose
    u32 sizes[3]; // icon.bin, banner.bin, sound.bin
    u32 flag1; // unknown
    u16 names[7][2][21]; // JP, EN, DE, FR, ES, IT, NL, stored as UTF16
    u8 zeroes_2[0x348]; // padding
    u8 crypto[16]; // MD5 of 0x40 to 0x640 in header (for 00000000.app, Opening.bnr is 0x80 to 0x680). 
                      // crypto should be all 0's when calculating final MD5
} __attribute__((packed)) IMET;

//Prototypes
s32 __makeContentPath(u64 tid, char* outASCII, size_t oaSize);
s32 __findIMETinTitle(u64 tid, IMET* outIMET);
s32 __getNameInIMET(IMET inIMET, u16* outUnicode, size_t ouSize, title_lang language, int line);
s32 __makeGenericName(u64 tid, u16* outUnicode, size_t ouSize, title_lang language, int line);

s32 __makeContentPath(u64 tid, char* outASCII, size_t oaSize) {
    char filename[256];
    sprintf(filename, "/title/%08x/%08x/content/", TITLE_UPPER(tid), TITLE_LOWER(tid), cid);
    memcpy(outASCII, filename, oaSize);
}

s32 __findIMETinTitle(u64 tid, IMET* outIMET) {
    s32 errno;
    
    //Get list of contents
    char* dirent_buffer;
    u32 dirent_length;

    //

    s32 fd = ISFS_Open(filename, ISFS_OPEN_READ);
    IMET* malignedIMET = outIMET;
    if (fd >= 0) {
        if (!ISALIGNED(malignedIMET)) {
            malignedIMET = memalign(32, sizeof(IMET));
        }
        
        errno = ISFS_Read(fd, outIMET, sizeof(IMET));
        
        ISFS_Close(fd);
    } else {
        
    }
    
    if (errno < 0) {
    }
    
    if (malignedIMET != outIMET) {
        //memcpy into unaligned block
        memcpy(outIMET, malignedIMET, sizeof(IMET));
    }
    
    
}
