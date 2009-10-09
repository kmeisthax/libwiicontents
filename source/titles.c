/*-------------------------------------------------------------
 
titles.c -- functions for grabbing all titles of a certain type
 
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

#include <gccore.h>
#include <ogcsys.h>

#include "titles.h"

#define MAX_TITLES 256
u32 __titles_init = 0;
u32 __num_titles;
static u64 __title_list[MAX_TITLES] ATTRIBUTE_ALIGN(32);

s32 __getTitles() {
	s32 ret;
	ret = ES_GetNumTitles(&__num_titles);
	if (ret <0)
		return ret;
	if (__num_titles > MAX_TITLES)
		return -1;
	ret = ES_GetTitles(__title_list, __num_titles);
	if (ret <0)
		return ret;
	__titles_init = 1;
	return 0;
}

s32 getTitles_TypeCount(u32 type, u32 *count) {
	s32 ret = 0;
	u32 type_count;
	if (!__titles_init)
		ret = __getTitles();
	if (ret <0)
			return ret;
	int i;
	type_count = 0;
	for (i=0; i < __num_titles; i++) {
		u32 upper;
		upper = __title_list[i] >> 32;
		if(upper == type)
			type_count++;
	}
	*count = type_count;
	return ret;
}
	
s32 getTitles_Type(u32 type, u32 *titles, u32 count) {
	s32 ret = 0;
	u32 type_count;
	if (!__titles_init)
		ret = __getTitles();
	if (ret <0)
			return ret;
	int i;
	type_count = 0;
	for (i=0; type_count < count && i < __num_titles; i++) {
		u32 upper, lower;
		upper = __title_list[i] >> 32;
		lower = __title_list[i] & 0xFFFFFFFF;
		if(upper == type) {
			titles[type_count]=lower;
			type_count++;
		}
	}
	if (type_count < count)
		return -2;
	__titles_init = 0;
	return 0;
}
