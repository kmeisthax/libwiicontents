/*-------------------------------------------------------------
 
ioscheck.c - Check the status of the currently running IOS

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

#include <ogc/es.h>

#include "wc_private.h"
#include "wiicontents.h"

typedef struct
{
	u32 id;
	u16 index;
	u16 type;
	u64 size;
} __attribute__((packed)) tmdview_content;
 
typedef struct
{
    u8 version; // 0x0000;
	u8 filler[3];
	u64 ios_title_id; //0x0004
	u64 title_id; // 0x00c
	u32 title_type; //0x0014
	u16 group_id; //0x0018
	u8 reserved[0x3e]; //0x001a this is the same reserved 0x3e bytes from the tmd
	u16 title_version; //0x0058
	u16 number_contents; //0x005a
	tmdview_content contents[]; //0x005c
} __attribute__((packed)) tmdview;

s32 iosc_listInstalledIOS(u8* numIOS, u8* IOSnumArray, size_t maxEntries) {
    s32 out = 0;
    
    u8 internalNumIOS = 0;
    
    u32 titlesMax ALIGN_32 = 0;
    s32 err = ES_GetNumTitles(&titlesMax);
    
    u64* titlesArray = malloc(titlesMax * sizeof(u64));
    if (titlesArray == NULL) {
        out = WCT_ENOMEM;
        goto finish_up;
    }
    
    for (int i = 0; i < titlesMax && i < maxEntries; i++) {
        if (TITLE_UPPER(titlesArray[i]) == 0x00000001 &&
            TITLE_LOWER(titlesArray[i]) < 0x000000FF &&
            TITLE_LOWER(titlesArray[i]) > 0x00000002) {
            //It's an IOS
            
            if (IOSnumArray != NULL && internalNumIOS < maxEntries) {
                IOSnumArray[internalNumIOS] = (u8)titlesArray[i];
            }
            internalNumIOS += 1;
        }
    }
    
    if (numIOS != NULL) {
        *numIOS = internalNumIOS;
    }
    
    dealloc_titlesArray:
    free(titlesArray);
    
    finish_up:
    return out;
}

s32 iosc_testIOS(u8 slot, iosc_manifest* info) {
    s32 out = 0;
    
    //Produce the proper title ID
    u64 tid ALIGN_32 = slot;
    
    //Get TMDView struct size
    u32 bytes_needed ALIGN_32 = 0;
    s32 err = ES_GetTMDViewSize(tid, &bytes_needed);
    if (err < 0) {
        switch (err) {
            case ES_ENOMEM:
                out = WCT_ENOMEM;
                break;
            case ES_EINVAL:
                out = WCT_EINTERNAL;
                break;
            case ES_EALIGN:
                out = WCT_ENOTALIGNED;
                break;
            case ES_ENOTINIT:
                out = WCT_EIOSNOTINIT;
                break;
        }
        goto finish_up;
    }
    
    finish_up:
    return out;
}
