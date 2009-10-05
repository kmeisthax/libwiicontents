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
#include "wc_private.c"
#include "strlcpy.c"
#include "strlcat.c"

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

//Constants
#define IMET_MAGIC 0x494D4554 //"IMET"

//Prototypes
s32 __makeContentPath(u64 tid, char* outASCII, size_t oaSize);
s32 __findIMETinTitle(u64 tid, u32* cid, IMET* outIMET);
s32 __getNameInIMET(IMET inIMET, u16* outUnicode, size_t ouSize, title_lang language, int line);
s32 __makeGenericName(u64 tid, u16* outUnicode, size_t ouSize, title_lang language, int line);

s32 __makeContentPath(u64 tid, char* outASCII, size_t oaSize) {
    char filename[255];
    snprintf(filename, 255, "/title/%08x/%08x/content/", TITLE_UPPER(tid), TITLE_LOWER(tid), cid);
    strlcpy(outASCII, filename, oaSize);
    return WCT_OKAY;
}

s32 __findIMETinTitle(u64 tid, u32* cid, IMET* outIMET) {
    s32 errno;
    IMET malignedIMET ALIGN_32;
    
    //Make title directory string
    char filename[255] ALIGN_32;
    __makeContentPath(tid, filename, 255);
    
    //Get list of contents
    u32 dirents = 0;
    ISFS_ReadDir(__makeContentPath(), NULL, &dirents);
    char* dirent_buffer = memalign(32, dirents * ISFS_DIRENT_SIZE);
    ISFS_ReadDir(__makeContentPath(), dirent_buffer, &dirents);

    int i = 0;
    int bannerfound = false;

    for (i = 0; i < dirents; i++) {
        char* cur_dirent = dirent_buffer[ISFS_DIRENT_SIZE * i];
        char cur_filename[255] ALIGN_32;

        strlcpy(cur_filename, filename, 255);
        strlcat(cur_filename, cur_dirent, 255);
        
        s32 fd = ISFS_Open(cur_filename, ISFS_OPEN_READ);
        if (fd >= 0) {
            errno = ISFS_Read(fd, malignedIMET, sizeof(IMET));
            ISFS_Close(fd);
            //Check to see if this is a valid IMET
            if (malignedIMET->imet == IMET_MAGIC) {
                //Found the IMET!
                int bannerfound = true;
                hex2u32(cur_dirent, cid);
                break;
            }
        }
    }
    
    if (bannerfound) {
        //memcpy into unaligned block
        memcpy(outIMET, malignedIMET, sizeof(IMET));
    }
    
    //free our memory
    free(dirent_buffer);
}
