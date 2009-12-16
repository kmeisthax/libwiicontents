/*-------------------------------------------------------------
 
wiicontents.c - easy access to channel data

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

#ifndef __WCT_INCLUDE_H__
#define __WCT_INCLUDE_H__

#include <gctypes.h>

//Operation successful.
#define WCT_OKAY             0
//A memory buffer you gave was not properly aligned.
//Generally, this won't happen since libwct copies memory on the fly into aligned buffers.
//If you get this error code, there is a bug in libwct.
#define WCT_ENOTALIGNED     -1
//A memory buffer you gave wasn't big enough.
#define WCT_EOVERFLOW       -2
//Ran out of memory.
#define WCT_ENOMEM          -3
//There's no banner in this particular title.
#define WCT_ENOBANNER       -4
//You sent an empty buffer, or a NULL pointer
#define WCT_EBADBUFFER      -5
//Initialization failed.
#define WCT_EISFSFAIL       -6
//ES_Identify privledge escalation failed. (May or may not mean patched IOS!)
#define WCT_ENOIDENTIFY     -7
//You failed to initialize a common IOS subsystem that should be initialized (like ES)
#define WCT_EIOSNOTINIT     -8
//IOS hates you. Specifically, it threw up an EINVAL code of some kind.
#define WCT_EINTERNAL       -9
//An IOS subsystem threw up an unknown error code (May indicate broken IOS!)
#define WCT_EUNKNOWN        -10
//The file you gave is too small and doesn't have the needed data.
#define WCT_EFILETOOSMALL   -11
//You're an idiot.
#define WCT_ECIRNO         0x2468


//Useful constants.
//These are possible languages.
typedef enum {
    lang_en, lang_jp, lang_de, lang_fr, lang_es, lang_it, lang_nl
} WCT_title_lang;

//These are possible devices.
typedef enum {
    device_any, device_nand, device_sd
} WCT_device_type;

//This determines what 'mode' we are running in.
//Trying to initialize in ios_su, superuser mode, will use the ES_Identify bug if possible.
//ios_game assumes the restrictions of a game.
typedef enum {
    execution_ios_su, execution_ios_game
} WCT_execution_context;

//Init functions.
s32 WCT_Init(); //Initialize contents access, and attempt to exploit ES_Identify.
s32 WCT_Deinit(); //Deinitialize WCT, which deinits ISFS.

//Title mounting functions. device_type will tell the system if you want to mount title data/contents from SD or NAND.
s32 WCT_MountTitleContents(u64 tid, WCT_device_type device);
s32 WCT_MountTitleData(u64 tid, WCT_device_type device);

//Title name retrieving functions.
//These functions will generate a somewhat useful name for any title.
//If the title has a banner, we use that.
//Names are split into two lines, only TitleName is shown on the menu.
//TitleTagline is shown as the 2nd line on the Wii Message Board.
s32 WCT_TitleNameUTF16(u16* outstring, u32 length, u64 tid, WCT_title_lang language);
s32 WCT_TitleNameASCII(char* outstring, u32 length, u64 tid, WCT_title_lang language);
s32 WCT_TitleTaglineUTF16(u16* outstring, u32 length, u64 tid, WCT_title_lang language);
s32 WCT_TitleTaglineASCII(char* outstring, u32 length, u64 tid, WCT_title_lang language);

#endif
