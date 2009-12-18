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

#ifndef __WCT_IOSCHECK_H__
#define __WCT_IOSCHECK_H__

#include <gccore.h>

//A list of IOS capabilities bits.
//IOSC_IDENTIFY IOSes let you call ES_Identify.
#define IOSC_IDENTIFY   0x00000001
//IOSC_DEVFLASH IOSes let you read/write /dev/flash.
#define IOSC_DEVFLASH   0x00000002
//IOSC_TRUCHA IOSes let you install content with forged signatures
//(see http://wiibrew.org/wiki/Trucha )
#define IOSC_TRUCHA     0x00000004
//IOSC_UNSIGNED IOSes have a forged TMD signature.
#define IOSC_UNSIGNED   0x00000008
//IOSC_USB2 IOSes support USB2 mode.
#define IOSC_USB2       0x00000010
//IOSC_BOOTMII IOSes are instances of BootMii/IOS. (Also impl. IOSC_UNSIGNED)
#define IOSC_BOOTMII    0x00000020
//IOSC_CIOS IOSes are known Waninkoko releases. (Also impl. IOSC_UNSIGNED)
#define IOSC_CIOS       0x00000040
//IOSC_SOFTMOD IOSes will read Wii game data off of backup media.
#define IOSC_SOFTMOD    0x00000080
//IOSC_STUB IOSes are nonfunctional IOSes installed by Nintendo.
//They are usually installed to known cIOS or PatchMii locations to thwart
//backups and homebrew. Do not reboot to this IOS!
#define IOSC_STUB       0x00000100

typedef u32 iosc_caps;

typedef struct {
    u64 tid;
    u8 ios_num;
    iosc_caps capabilities;
} iosc_manifest

s32 iosc_countInstalledIOS(u8* numIOS);
s32 iosc_listInstalledIOS(u8* IOSnumArray, size_t maxEntries);
s32 iosc_testIOS(u8 slot, iosc_manifest* info);

#endif
