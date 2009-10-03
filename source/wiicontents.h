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

//Useful constants.
typedef enum {
    lang_en, lang_jp, lang_de, lang_fr, lang_es, lang_it, lang_nl
} title_lang;

//Init functions.
s32 WCT_Init(); //Initialize contents access, and attempt to exploit ES_Identify.
s32 WCT_Deinit(); //Deinitialize WCT, which deinits ISFS.

//Title contents functions.
s32 WCT_TitleSaveDir(char* outstring, u32 length, u64 tid);
s32 WCT_TitleContentDir(char* outstring, u32 length, u64 tid);

//Title name retrieving functions.
//These functions will generate a somewhat useful name for any title.
//Names are split into two lines, only TitleName is shown on the menu.
//TitleTagline is shown as the 2nd line on the Wii Message Board.
s32 WCT_TitleNameUTF16(u16* outstring, u32 length, u64 tid, title_lang language);
s32 WCT_TitleNameASCII(char* outstring, u32 length, u64 tid, title_lang language);
s32 WCT_TitleTaglineUTF16(u16* outstring, u32 length, u64 tid, title_lang language);
s32 WCT_TitleTaglineASCII(char* outstring, u32 length, u64 tid, title_lang language);
