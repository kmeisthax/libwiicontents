/*-------------------------------------------------------------
 
wiibasics.c -- basic Wii initialization and functions
 
Copyright (C) 2008 tona
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

#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "wc_private.h"
#include "wiicontents.h"
#include "wiibasics.h"
#include "id.h"
#include "uninstall.h"

s32 miscInit(void)
{
	s32 ret;

	ret = Identify_SU();
	if (ret < 0){
		//wait_anyKey();
		return WCT_ENOIDENTIFY;
	}
	
	//printf("\n\tInitializing Filesystem driver...");
	fflush(stdout);
	
	ret = ISFS_Initialize();
	if (ret < 0) {
	    return WCT_EISFSFAIL;
		//printf("\n\tError! ISFS_Initialize (ret = %d)\n", ret);
		//wait_anyKey();
	} //else {
		//printf("OK!\n");
	//}
	
	//printf("\tWiping off fingerprints...\n");
	fflush(stdout);
	Uninstall_FromTitle(TITLE_ID(1, 0));
	
	return WCT_OKAY;
}

s32 miscDeInit(void)
{
	fflush(stdout);
	ISFS_Deinitialize();
	
	return WCT_OKAY;
}

u32 getButtons(void)
{
	WPAD_ScanPads();
	return WPAD_ButtonsDown(0);
}

u32 wait_anyKey(void) {
	u32 pressed;
	while(!(pressed = getButtons())) {
		VIDEO_WaitVSync();
	}
	return pressed;
}

u32 wait_key(u32 button) {
	u32 pressed;
	do {
		VIDEO_WaitVSync();
		pressed = getButtons();
	} while(!(pressed & button));
	return pressed;
}
