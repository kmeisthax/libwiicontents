/*-------------------------------------------------------------
 
sddata.c - read SD savegame data

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

#include <gccore.h>

#include "wc_private.h"
#include "sddata.h"

typedef struct {
    u32 magic; //WIBN
    u32 reserved;
    u32 flags;
    u8[20] reserved2;
    u16[2][32] name_strings;
    sddata_banner banner_tex;
    sddata_icon[] banner_icons;
} __attribute__((packed)) DataBinBanner;

typedef struct {
    u64 savegameID;
    u32 banner_size;
    u8 permissions;
    u8 unknown;
    u8[16] md5_bkheader;
    u16 unknown_2;
} __attribute__((packed)) DataBinHeader;

typedef struct {
    u32 bkheader_size;
    u32 magic; //BK 0x0001
    u32 ng-id;
    u32 numfiles;
    u32 files_size;
    u32 unknown;
    u32 unknown2;
    u32 total_size;
    u8[64] unknown3;
    u32 unknown4;
    u32 game_id;
    u8[6] mac_addr;
    u16 unknown5;
    u8[16] zeroes;
} __attribute__((packed)) DataBinBKHeader;

typedef enum {
    databin_file = 1, databin_dir = 2
} DataBinFiletypeEnum;

typedef struct {
    u32 magic; //0x03adf17e
    u32 filesize;
    u8 permissions;
    u8 attributes;
    u8 filetype;
    u8[0x45] name;
    u8[16] iv;
    u8[0x20] unknown;
} __attribute__((packed)) DataBinFileHeader;

s32 __dbin_decode(DataBin* context) {
    //Finish up context initialization.
    context->mounted = FALSE;
    context->devicename = NULL;
    
    FILE* fd = context->fd;
    long int oldfseek = ftell(fd);
    
    //Setup the header offset/lengths
    int ptr = 0;
    int len = sizeof(DataBinHeader);
    
    context->offsets.header = ptr;
    context->lengths.header = len;
    
    //Read the header
    DataBinHeader dbh;
    DataBinHeader dbh_clear;
    
    if (fseek(fd, ptr, SEEK_SET) < 0) {
        switch (errno) {
            case EINVAL:
                out = WCT_EFILETOOSMALL;
                break;
            default:
                out = WCT_EUNKNOWN;
        }
        goto finish_up;
    }
    
    if (fread(fd, len, 1, &dbh) < len) {
        out = WCT_EFILETOOSMALL;
        goto finish_up;
    }
    
    //Decrypt the header
    s32 err = ES_Decrypt(ES_KEY_SDCARD, sd_iv, &dbh, len, &dbh_clear);
    if (err < 0) {
        switch (err) {
            case ES_ENOMEM:
                out = WCT_ENOMEM;
                break;
            case ES_ENOTINIT:
                out = WCT_EIOSNOTINIT;
                break;
            default:
                out = WCT_EINTERNAL;
        }
        goto finish_up;
    }
    
    //Read the banner data
    ptr += len;
    len = dbh.banner_size;
    
    context->offsets.header = ptr;
    context->lengths.header = len;
    
    DataBinBanner* dbb = malloc(len);
    if (dbb == NULL) {
        out = WCT_ENOMEM;
        goto finish_up;
    }
    
    if (fseek(fd, ptr, SEEK_SET) < 0) {
        switch (errno) {
            case EINVAL:
                out = WCT_EFILETOOSMALL;
                break;
            default:
                out = WCT_EUNKNOWN;
        }
        goto dealloc_databinbanner;
    }
    
    if (fread(fd, len, 1, &dbh) < len) {
        out = WCT_EFILETOOSMALL;
        goto dealloc_databinbanner;
    }
    
    //Read the backup header
    ptr += len;
    len = 
    
    dealloc_databinbanner:
    free(dbb);
    
    finish_up:
    fseek(oldfseek);
    return out;
}

s32 dbin_openfd(FILE* fd, DataBin* context) {
    s32 out = 0;
    
    if (fd == NULL || context == NULL) {
        out = WCT_EBADBUFFER;
        goto finish_up;
    }
    
    context->fd = fd;
    context->close_fd = FALSE;
    
    out = __dbin_decode(context);
    
    finish_up:
    return out;
}

s32 dbin_open(const char* filename, DataBin* context) {
    s32 out = 0;
    
    if (context == NULL) {
        out = WCT_EBADBUFFER;
        goto finish_up;
    }
    
    context->close_fd = TRUE;
    FILE* fd = fopen(filename, "r");
    if (fd == NULL) {
        switch (errno) { //I hate errno.
            case ENOMEM:
                out = WCT_ENOMEM;
            default:
                out = WCT_EUNKNOWN;
        }
        
        goto finish_up;
    }
    
    out = dbin_decode(context);
    if (out < 0)
        fclose(fd);
    
    finish_up:
    return out;
}
