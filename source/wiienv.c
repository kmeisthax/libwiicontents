/*-------------------------------------------------------------
 
wiienv.c - obtain information from the Wii environment

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

#include "wiienv.h"
#include "wc_private.h"
#include "wiicontents.h"

//might as well be include <vulnerabilities.h>
#include <string.h>

#include <ogc/es.h>

//useful addresses
#define STUBHAXX_PTR 0x80001804
#define TID_LOWER_PTR  0x80003180

s32 __runningFromLoader(int* trueIfLoader) {
    //Check for the presence of "STUBHAXX" at STUBHAXX_PTR
    char* stubhaxx = (char*)STUBHAXX_PTR;
    char* testhaxx = "STUBHAXX";
    
    if (memcmp(stubhaxx, testhaxx, 8) == 0)
        *trueIfLoader = true;
    else
        *trueIfLoader = false;
        
    return WCT_OKAY;
}

s32 __lowerTidFromMemory(u32* outLowTid) {
    u32* sysTid = (u32*)TID_LOWER_PTR;
    
    *outLowTid = sysTid;
}

s32 __TidFromES(u64* outTid) {
    u64* sysTid ALIGN_32;
    s32 ret = ES_GetTitleID(&sysTid);
    
    *outTid = *sysTid;
    return ret
}
