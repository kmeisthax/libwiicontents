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
 
A good number of support functions, mostly relating to exploiting
ES_Identify, were borrowed from AnyTitle Deleter.
-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/es.h>
#include <ogc/ios.h>

#include "wiicontents.h"

#include "../build/certs_dat.h"
#include "../build/fake_su_tmd_dat.h"
#include "../build/fake_su_ticket_dat.h"

#include "detect_settings.h"
#include "titles.h"
#include "uninstall.h"
#include "wiibasics.h"

s32 WCT_Init() {
	return miscInit();
}

s32 WCT_Deinit() {
    return miscDeInit();
}
