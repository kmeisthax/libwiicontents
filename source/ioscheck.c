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
    s32 err = ES_GetStoredTMDSize(tid, &bytes_needed);
    if (err < 0) {
        convertEStoWCTError(&err);
        out = err;
        
        goto finish_up;
    }
    
    //Allocate a signed blob
    signed_blob* stmnd = memalign(32, bytes_needed);
    if (iosdata == NULL) {
        out = WCT_ENOMEM;
        goto finish_up;
    }
    
    //Fill it with data
    err = ES_GetStoredTMD(tid, stmnd, bytes_needed);
    if (err < 0) {
        convertEStoWCTError(&err);
        out = err;
        
        goto finish_up;
    }
    
    //Get the actual IOS structure.
    tmd* ios_meta = (tmd*)SIGNATURE_PAYLOAD(stmnd);
    
    //Do some testing on the TMD.
    
    dealloc_tmdview:
    free(stmnd);
    
    finish_up:
    return out;
}
