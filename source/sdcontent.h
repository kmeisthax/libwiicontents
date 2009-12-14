/*-------------------------------------------------------------
 
sdcontent.h - Read data from SD content.bin files

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
 
A good number of support functions, mostly relating to exploiting
ES_Identify, were borrowed from AnyTitle Deleter.
-------------------------------------------------------------*/

#ifndef __WCT_SDCONTENT_H__
#define __WCT_SDCONTENT_H__

#include <gccore.h>
#include <ogc/es.h>

#include "titlegen.h"
#include "wiicontents.h"

typedef enum {
    CBIN_FILE, CBIN_BUFFER
} ContentBinType;

typedef struct {
    WCT_execution_context mode;
    ContentBinType type;
    union {
        int fd;
        char* buffer;
    } datasource;
    struct {
        int header;
        int icon;
        int bk;
        int tmd;
        int contents;
        int certs;
    } offsets;
} ContentBin;

//Basic initialization, sanity testing
//Call this for every mode you plan to use.
s32 cbin_init(ContentBinMode mode);
s32 cbin_deinit(ContentBinMode mode);

//Opening and closing ContentBin contexts
s32 cbin_open(const char* filename, ContentBin* context, WCT_execution_context mode);
s32 cbin_openfd(int fd, ContentBin* context, WCT_execution_context mode);
s32 cbin_openbuf(const char* buffer, ContentBin* context, WCT_execution_context mode);
s32 cbin_close(ContentBin* context);

//Retrieving the embedded icon file in the Content.bin file.
s32 cbin_iconbinsize(ContentBin* context, size_t* bufSize);
s32 cbin_iconbinread(ContentBin* context, u8* buffer, size_t bufSize);

s32 cbin_getname(ContentBin* context, u16* outUnicode, size_t ouSize, title_lang language, int line);

#endif
