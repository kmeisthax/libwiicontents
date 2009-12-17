/*-------------------------------------------------------------
 
sddata.h - read SD savegame data

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

#ifndef __WCT_SDDATA_H__
#define __WCT_SDDATA_H__

#include <stdio.h>
#include <gccore.h>

typedef u8[24576] sddata_banner;
typedef u8[4608] sddata_icon;

typedef struct {
    FILE* fd;
    int close_fd;
    int mounted;
    const char* devicename;
    struct {
        int header;
        int banner;
        int bk;
        int files;
    } offsets;
    struct {
        int header;
        int banner;
        int bk;
        int files;
    } lengths;
} DataBin;

s32 dbin_open(const char* filename, DataBin* context);
s32 dbin_openfd(FILE fd, DataBin* context);

s32 dbin_close(DataBin* context);

s32 dbin_getBanner(DataBin* context, sddata_banner* outBanner);
s32 dbin_getNumIcons(DataBin* context, int* outnum);
s32 dbin_getIcon(DataBin* context, int icon_id, sddata_icon* outIcon);
s32 dbin_getNameUTF16(DataBin* context, int name_line, u16* outName, size_t numChars);
s32 dbin_getInternalGameID(DataBin* context, u32* outID);
s32 dbin_getWiiNGKey(DataBin* context, u32* outNGID);

s32 dbin_mountSave(DataBin* context, const char* devicename);
s32 dbin_unmountSave(DataBin* context);
#endif
