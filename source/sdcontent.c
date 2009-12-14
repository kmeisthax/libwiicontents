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
