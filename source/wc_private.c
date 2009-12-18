/*-------------------------------------------------------------
 
wc_private.c - wiicontents internal functions

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

#include "wc_private.h"

#include <math.h>
#include "wiicontents.h"

//Annoying defines so you can switch on letters.
#define ASCII_0 0x30
#define ASCII_9 0x39
#define ASCII_A 0x41
#define ASCII_F 0x46

void hex2u32(const char* inhex, u32* out32) {
    //Assume big endian representation
    int i = 0;
    u32 cur32 = 0;
    for (; i < 8; i++) {
        u8 ltr = (u8)inhex[i];
        int pwr = 7 - i;
        
        if (ltr <= ASCII_9 && ltr >= ASCII_0) {
            int mag = ltr - ASCII_0;
            cur32 = cur32 + mag * pow(16, pwr);
        } else if (ltr <= ASCII_F && ltr >= ASCII_A) {
            int mag = (ltr - ASCII_A) + 10;
            cur32 = cur32 + mag * pow(16, pwr);
        }
    }
    
    *out32 = cur32;
}

//Convert strings to UTF16 strings.
s32 str2u16(u16* outUtf16, size_t length, const char* inAscii, size_t* copied) {
    if (outUtf16 == NULL || inAscii == NULL || length < 1) {
        return WCT_EBADBUFFER;
    }
    
    const char* curChar = inAscii;
    u16* curUtf16 = outUtf16;
    
    int i = 0;

    while (true) {
        //Buffer edge check. Avoid overflows in style.
        if ((i + 1) == length) {
            //Write a nice null anyway.
            *curUtf16 = 0;
            break;
        }
        
        //Otherwise continue copying characters.
        *curUtf16 = (u16)*curChar;
        
        //Check if the byte we copied was NULL
        if ((u8)(*curChar) == 0) {
            //Done copying.
            break;
        }
        
        //Otherwise, move forward the pointers
        curChar++;
        curUtf16++;
        i++;
    }
    
    if (copied != NULL) { //this parameter is optional.
        *copied = i;
    }
    
    return WCT_OKAY;
}

//Convert ES error messages to WCT error messages
s32 convertEStoWCTError (s32* errorcode) {
    s32 = out;
    
    switch (err) {
        case ES_ENOMEM:
            out = WCT_ENOMEM;
            break;
        case ES_EINVAL:
            out = WCT_EINTERNAL;
            break;
        case ES_EALIGN:
            out = WCT_ENOTALIGNED;
            break;
        case ES_ENOTINIT:
            out = WCT_EIOSNOTINIT;
            break;
        default:
            out = WCT_EUNKNOWN;
            break;
    }
    
    return out;
}
