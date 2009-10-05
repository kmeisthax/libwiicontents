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

#ifndef __WC_PRIVATE_C__
#define __WC_PRIVATE_C__

#include <stdlib.h>
#include <math.h>

//From AnyTitle Deleter
// Turn upper and lower into a full title ID
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
// Get upper or lower half of a title ID
#define TITLE_UPPER(x)		((u32)((x) >> 32))
// Turn upper and lower into a full title ID
#define TITLE_LOWER(x)		((u32)(x))

#define ISALIGNED(x) ((((u32)x)&0x1F)==0)
#define ALIGN_32    __attribute__ ((aligned (32)))

//A dirent is the size of a DOS filename plus . and \0. 8 + 1 + 3 + 1 = 13
#define ISFS_DIRENT_SIZE 13

void* memalign(unsigned int, unsigned int);
void hex2u32(const char* inhex, u32* out32);

void hex2u32(const char* inhex, u32* out32) {
    //Assume big endian representation
    int i = 0;
    for (; i < 8; i++) {
        const char* ltr = inhex[i];
        int pwr = 7 - i;

        switch (*ltr) {
            case "0":
                break;
            case "1":
                out32* += 1 * pow(16, pwr);
                break;
            case "2":
                out32* += 2 * pow(16, pwr);
                break;
            case "3":
                out32* += 3 * pow(16, pwr);
                break;
            case "4":
                out32* += 4 * pow(16, pwr);
                break;
            case "5":
                out32* += 5 * pow(16, pwr);
                break;
            case "6":
                out32* += 6 * pow(16, pwr);
                break;
            case "7":
                out32* += 7 * pow(16, pwr);
                break;
            case "8":
                out32* += 8 * pow(16, pwr);
                break;
            case "9":
                out32* += 9 * pow(16, pwr);
                break;
            case "A":
                out32* += 10 * pow(16, pwr);
                break;
            case "B":
                out32* += 11 * pow(16, pwr);
                break;
            case "C":
                out32* += 12 * pow(16, pwr);
                break;
            case "D":
                out32* += 13 * pow(16, pwr);
                break;
            case "E":
                out32* += 14 * pow(16, pwr);
                break;
            case "F":
                out32* += 15 * pow(16, pwr);
                break;
        }
    }

    return 0;
}

#endif
