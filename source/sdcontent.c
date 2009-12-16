/*-------------------------------------------------------------
 
sdcontent.c - Read data from SD content.bin files

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

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

#include "wcprivate.h"
#include "titlegen.h"
#include "sdcontent.h"

//The magic number for the BK-header.
#define BKH_MAGIC 0x426B0001

typedef struct {
    u64 titleid;
    u32 icon_size;
    u8[16] md5_header;
    u8[16] md5_icon;
    u32 unknown;
    u64[2] unknown_tids;
    u8[64] zeroes1;
    IMETCore banner_header;
    u8[0x358] padding;
} __attribute__((packed)) ContentBinHeader;

typedef struct {
    u32 size;
    u32 magic; //always equals BKH_MAGIC
    u32 ng-id;
    u8[8] zeroes;
    u32 tmd_size;
    u32 contents_size;
    u32 cbin_data_size; //Size of total file minus ContentBinHeader and the icons.
    u32[16] included_contents;
    u64 title_id;
    u8[8] unknown;
} __attribute__((packed)) ContentBinBKHeader;

//Read a number of bytes, write them to the out buffer.
//In the event that a NULL outbuf is given, this function will still write the amount of bytes that would have been written.
//This will not be a minimal number in the case of fread.
s32 __cbin_read(ContentBin* context, long int startloc, long int bytes, u8* outbuf, size_t bufsize, long int* writtenbytes) {
    s32 out = 0;
    
    long int towrite = 0;
    
    if (bytes > bufsize)
        towrite = bufsize;
    else if (bufsize > bytes)
        towrite = bytes;
    
    if (context->type == CBIN_FILE && outbuf != NULL) {
        long int original_pos = ftell(context->datasource.fd);
        fseek(startloc, SEEK_SET);
        towrite = fread(outbuf, towrite, 1, context->datasource.fd);
        fseek(original_pos, SEEK_SET);
    } else if (context->type == CBIN_BUFFER) {
        if ((startloc + towrite) > context->datasource.cb.l) {
            towrite = context->datasource.cb.l - startloc;
        }
        
        u8* start = context->datasource.cb.b + startloc;
        
        if (outbuf != NULL)
            memcpy(outbuf, start, towrite);
    }
    
    if (writtenbytes != NULL)
        *writtenbytes = towrite;
    
    return out;
}

//Read a number of bytes, decrypt them, and write them to the out buffer.
//The amount of bytes written is stored in writtenbytes.
//The actual decrypted data is stored in outbuf.
//Either of them are allowed to be null (Or both, but that would be horribly stupid.)
//You are allowed to pass in a buffer smaller than the amount of data you intend to read, though this is not recommended.
s32 __cbin_decrypted_read(ContentBin* context, long int startloc, long int bytes, u8* outbuf, size_t bufsize, long int* writtenbytes) {
    s32 out = 0;
    
    u8* cyphertext = memalign(32, bytes);
    if (cyphertext == NULL) {
        out = WCT_ENOMEM;
        goto finish_up;
    }
    
    u8* plaintext = memalign(32, bytes);
    if (plaintext == NULL) {
        out = WCT_ENOMEM;
        goto dealloc_cyphertext;
    }
    
    long int written = 0;
    
    s32 in = __cbin_read(context, startloc, bytes, cyphertext, bufsize, &written);
    if (in < 0) {
        out = in;
        goto dealloc_plaintext;
    }
    
    s32 err = ES_Decrypt(ES_KEY_SDCARD, sd_iv, cyphertext, written, plaintext);
    
    switch (err) {
    case ES_ENOMEM:
        out = WCT_ENOMEM;
        goto dealloc_plaintext;
    case ES_ENOTINIT:
        out = WCT_EIOSNOTINIT;
        goto dealloc_plaintext;
    case ES_EINVAL:
        out = WCT_EINTERNAL;
        goto dealloc_plaintext;
    case ES_EALIGN:
        out = WCT_ENOTALIGNED;
        goto dealloc_plaintext;
    default:
        if (err < 0) {
            out = WCT_EUNKNOWN;
            goto dealloc_plaintext;
        }
    }
    
    size_t tocopy;
    
    if (written < bufsize)
        tocopy = written;
    else if (bufsize < written)
        tocopy = bufsize;

    if (outbuf != NULL)        
        memcpy(outbuf, plaintext, tocopy);

    if (writtenbytes != NULL)    
        *writtenbytes = tocopy;
    
    dealloc_plaintext:
    free(plaintext);
    
    dealloc_cyphertext:
    free(cyphertext);
    
    finish_up:
    return out;
}

s32 __cbin_decode(ContentBin* context) {
    s32 out = 0;
    
    int ptr = 0;
    int len = 0x0640;
    
    context->offsets.header = ptr; //obvious
    context->lengths.header = len; //also obvious.
    
    ContentBinHeader head;
    long int written = 0;
    
    __cbin_decrypted_read(context, context->offsets.header, context->lengths.header, (u8*)&head, sizeof(ContentBinHeader), &written);
    
    if (written < context->lengths.header) {
        out = WCT_EFILETOOSMALL;
        goto finish_up;
    }
    
    ptr += context->lengths.header;
    len = head.icon_size;
    
    context->offsets.icon = ptr;
    context->lengths.icon = len;
    
    ptr += len + len mod 64;
    len = 0x70;
    
    context->offsets.bk = ptr;
    context->lengths.bk = len;
    
    ContentBinBKHeader bkhead;
    
    __cbin_read(context, context->offsets.bk, context->lengths.bk, (u8*)&bkhead, sizeof(ContentBinBKHeader), &written);
    
    int offsetsend = context.off
    
    finish_up:
    return out;
}

s32 cbin_open(const char* filename, ContentBin* context, WCT_execution_context mode) {
    FILE fp = fopen(filename, "r"); //This library does not support writing content.bin files.

    context->mode = mode;
    context->type = CBIN_FILE;
    context->datasource.fd = fd;
    
    return __cbin_decode(context);
}

s32 cbin_openfd(int fd, ContentBin* context, WCT_execution_context mode);
s32 cbin_openbuf(const char* buffer, ContentBin* context, WCT_execution_context mode);
s32 cbin_close(ContentBin* context);
