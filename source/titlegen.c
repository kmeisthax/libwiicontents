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

#include "titlegen.h"
#include "wiicontents.h"
#include "wc_private.h"
#include "strlcpy.h"
#include "strlcat.h"

s32 __makeContentPath(u64 tid, char* outASCII, size_t oaSize) {
    char filename[255];
    snprintf(filename, 255, "/title/%08x/%08x/content/", TITLE_UPPER(tid), TITLE_LOWER(tid));
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
    ISFS_ReadDir(filename, NULL, &dirents);
    char* dirent_buffer = memalign(32, dirents * ISFS_DIRENT_SIZE);
    ISFS_ReadDir(filename, dirent_buffer, &dirents);

    int i = 0;
    int bannerfound = false;

    for (i = 0; i < dirents; i++) {
        char* cur_dirent = dirent_buffer + (ISFS_DIRENT_SIZE * i);
        char cur_filename[255] ALIGN_32;

        strlcpy(cur_filename, filename, 255);
        strlcat(cur_filename, cur_dirent, 255);
        
        s32 fd = ISFS_Open(cur_filename, ISFS_OPEN_READ);
        if (fd >= 0) {
            errno = ISFS_Read(fd, (void*) &malignedIMET, sizeof(IMET));
            ISFS_Close(fd);
            //Check to see if this is a valid IMET
            if (malignedIMET.imet == IMET_MAGIC) {
                //Found the IMET!
                bannerfound = true;
                hex2u32(cur_dirent, cid);
                break;
            }
        }
    }
    
    if (bannerfound) {
        //memcpy into unaligned block
        memcpy(outIMET, &malignedIMET, sizeof(IMET));
    }
    
    //free our memory
    free(dirent_buffer);
    
    return WCT_OKAY;
}

s32 __getNameInIMET(IMET* inIMET, u16* outUnicode, size_t ouSize, title_lang language, int line) {
    u16* IMETstr = inIMET->names[language][line];
    size_t cpySize = ouSize;
    if (cpySize > 21)
        cpySize = 21; //to be safe
        
    memcpy(outUnicode, IMETstr, cpySize);
    
    return WCT_OKAY;
}

s32 __makeGenericName(u64 tid, u16* outUnicode, size_t ouSize, title_lang language, int line) {
    u32 tidtype = TITLE_UPPER(tid);
    u32 tidcode = TITLE_LOWER(tid);
    char* title;
    int free_title = false; //sometimes we do need to free(title).

    if (tidtype == 0x00000001) { //system software
        switch (tidcode) {
            case 0x00000001: //Boot2 tmd
                if (line == LINE_TITLE) {
                    title = "boot2";
                } else {
                    title = "System bootloader";
                }
                break;
            case 0x00000002: //Sysmenu
                if (line == LINE_TITLE) {
                    title = "System Menu";
                } else {
                    title = "Program Loader";
                }
                break;
            case 0x00000100: //BC
                if (line == LINE_TITLE) {
                    title = "BC";
                } else {
                    title = "Gamecube loader";
                }
                break;
            case 0x00000101: //BC
                if (line == LINE_TITLE) {
                    title = "MIOS";
                } else {
                    title = "Gamecube firmware";
                }
                break;
            default: //IOS?
                if (tidcode < 0x00000100 && tidcode > 0x00000002) {
                    //IOS!
                    if (line == LINE_TITLE) {
                        title = memalign(32, 21);
                        free_title = true;
                        snprintf(title, 21, "IOS%u", tidcode);
                    } else {
                        title = "System firmware";
                    }
                    break;
                } else {
                    if (line == LINE_TITLE) {
                        title = "Unknown (System)";
                    } else {
                        title = "System data";
                    }
                    break;
                }
                break;
        }
    } else { //Use the tidcode directly
        if (line == LINE_TITLE) {
            title = memalign(32, 21); //We have to use dynamic memory for this part
            free_title = true;
            snprintf(title, 21, "%08X-%08X", tidtype, tidcode);
        } else {
            title = "";
        }
    }

    s32 out = str2u16(outUnicode, ouSize, title, NULL);
    free(title);
    return out;
}
