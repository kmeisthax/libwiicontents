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

s32 Uninstall_FromTitle(const u64 tid);
void uninstall_checked(u32 kind, u32 title);

// Identify as the "super user"
s32 Identify_SU(void);

// Identify as the system menu
s32 Identify_SysMenu(void);

/* Debug functions adapted from libogc's es.c */
//#define DEBUG_ES
//#define DEBUG_IDENT

#ifdef DEBUG_IDENT
s32 __sanity_check_certlist(const signed_blob *certs, u32 certsize)
{
	int count = 0;
	signed_blob *end;
	
	if(!certs || !certsize) return 0;
	
	end = (signed_blob*)(((u8*)certs) + certsize);
	while(certs != end) {
#ifdef DEBUG_ES
		//printf("Checking certificate at %p\n",certs);
#endif
		certs = ES_NextCert(certs);
		if(!certs) return 0;
		count++;
	}
#ifdef DEBUG_ES
	//printf("Num of certificates: %d\n",count);
#endif
	return count;
}
#endif

/* Reads a file from ISFS to an array in memory */
s32 ISFS_ReadFileToArray (char *filepath, u8 *filearray, u32 max_size, u32 *file_size) {
	s32 ret, fd;
	static fstats filestats ATTRIBUTE_ALIGN(32);
	ret = ISFS_Open(filepath, ISFS_OPEN_READ);
	if (ret <= 0)
	{
		//printf("Error! ISFS_Open (ret = %d)\n", ret);
		return -1;
	}
	
	fd = ret;
	
	ret = ISFS_GetFileStats(fd, &filestats);
	if (ret < 0)
	{
		//printf("Error! ISFS_GetFileStats (ret = %d)\n", ret);
		return -1;
	}
	
	*file_size = filestats.file_length;
	
	if (*file_size > max_size)
	{
		//printf("File is too large! Size: %u", *file_size);
		return -1;
	}
	
	ret = ISFS_Read(fd, filearray, *file_size);
	if (ret < 0)
	{
		//printf("Error! ISFS_Read (ret = %d)\n", ret);
		return -1;
	}
	
	ret = ISFS_Close(fd);
	if (ret < 0)
	{
		//printf("Error! ISFS_Close (ret = %d)\n", ret);
		return -1;
	}
	return 0;
}

s32 Identify(const u8 *certs, u32 certs_size, const u8 *idtmd, u32 idtmd_size, const u8 *idticket, u32 idticket_size) {
	s32 ret;
	u32 keyid = 0;
	ret = ES_Identify((signed_blob*)certs, certs_size, (signed_blob*)idtmd, idtmd_size, (signed_blob*)idticket, idticket_size, &keyid);
	if (ret < 0){
		switch(ret){
			case ES_EINVAL:
				//printf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				//printf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				//printf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				//printf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				//printf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
#ifdef DEBUG_IDENT
		//printf("\tTicket: %u Std: %u Max: %u\n", idticket_size, STD_SIGNED_TIK_SIZE, MAX_SIGNED_TMD_SIZE);
		//printf("\tTMD: signed? %d\n", IS_VALID_SIGNATURE(idtmd));
		//printf("\tCerts: Sane? %d\n", __sanity_check_certlist(certs, certs_size));
		if (!ISALIGNED(certs)) //printf("\tCertificate data is not aligned!\n");
		if (!ISALIGNED(idtmd)) //printf("\tTMD data is not aligned!\n");
		if (!ISALIGNED(idticket)) //printf("\tTicket data is not aligned!\n");
#endif
	}
	else
		//printf("OK!\n");
	return ret;
}



s32 Identify_SU(void) {
	//printf("\n\tInforming Wii that I am God...");
	fflush(stdout);
	return Identify(certs_dat, certs_dat_size, fake_su_tmd_dat, fake_su_tmd_dat_size, fake_su_ticket_dat, fake_su_ticket_dat_size);
}

s32 Identify_SysMenu(void) {
	s32 ret;
	u32 sysmenu_tmd_size, sysmenu_ticket_size;
	static u8 sysmenu_tmd[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
	static u8 sysmenu_ticket[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(32);
	
	//printf("\n\tPulling Sysmenu TMD...");
	ret = ISFS_ReadFileToArray ("/title/00000001/00000002/content/title.tmd", sysmenu_tmd, MAX_SIGNED_TMD_SIZE, &sysmenu_tmd_size);
	if (ret < 0) {
		//printf("\tReading TMD failed!\n");
		return -1;
	}
	
	//printf("\n\tPulling Sysmenu Ticket...");
	ret = ISFS_ReadFileToArray ("/ticket/00000001/00000002.tik", sysmenu_ticket, STD_SIGNED_TIK_SIZE, &sysmenu_ticket_size);
	if (ret < 0) {
		//printf("\tReading TMD failed!\n");
		return -1;
	}
	
	//printf("\n\tInforming the Wii that I am its daddy...");
	fflush(stdout);
	return Identify(certs_dat, certs_dat_size, sysmenu_tmd, sysmenu_tmd_size, sysmenu_ticket, sysmenu_ticket_size);
}

/* Uninstall_Remove* functions taken from Waninkoko's WAD Manager 1.0 source */
s32 Uninstall_RemoveTitleContents(u64 tid)
{
	s32 ret;

	/* Remove title contents */
	//printf("\t\t- Removing title contents...");
	fflush(stdout);

	ret = ES_DeleteTitleContent(tid);
	if (ret < 0)
		//printf("\n\tError! ES_DeleteTitleContent (ret = %d)\n", ret);
	else
		//printf(" OK!\n");

	return ret;
}

s32 Uninstall_RemoveTitle(u64 tid)
{
	s32 ret;

	/* Remove title */
	//printf("\t\t- Removing title...");
	fflush(stdout);

	ret = ES_DeleteTitle(tid);
	if (ret < 0)
		//printf("\n\tError! ES_DeleteTitle (ret = %d)\n", ret);
	else
		//printf(" OK!\n");

	return ret;
}

s32 Uninstall_RemoveTicket(u64 tid)
{
	static tikview viewdata[0x10] ATTRIBUTE_ALIGN(32);

	u32 cnt, views;
	s32 ret;

	//printf("\t\t- Removing tickets...");
	fflush(stdout);

	/* Get number of ticket views */
	ret = ES_GetNumTicketViews(tid, &views);
	if (ret < 0) {
		//printf(" Error! (ret = %d)\n", ret);
		return ret;
	}

	if (!views) {
		//printf(" No tickets found!\n");
		return 1;
	} else if (views > 16) {
		//printf(" Too many ticket views! (views = %d)\n", views);
		return -1;
	}
	
	/* Get ticket views */
	ret = ES_GetTicketViews(tid, viewdata, views);
	if (ret < 0) {
		//printf(" \n\tError! ES_GetTicketViews (ret = %d)\n", ret);
		return ret;
	}

	/* Remove tickets */
	for (cnt = 0; cnt < views; cnt++) {
		ret = ES_DeleteTicket(&viewdata[cnt]);
		if (ret < 0) {
			//printf(" Error! (view = %d, ret = %d)\n", cnt, ret);
			return ret;
		}
	}
	//printf(" OK!\n");

	return ret;
}

s32 Uninstall_DeleteTitle(u32 title_u, u32 title_l)
{
	s32 ret;
	char filepath[256];
	s//printf(filepath, "/title/%08x/%08x",  title_u, title_l);
	
	/* Remove title */
	//printf("\t\t- Deleting title file %s...", filepath);
	fflush(stdout);

	ret = ISFS_Delete(filepath);
	if (ret < 0)
		//printf("\n\tError! ISFS_Delete(ret = %d)\n", ret);
	else
		//printf(" OK!\n");

	return ret;
}

s32 Uninstall_DeleteTicket(u32 title_u, u32 title_l)
{
	s32 ret;

	char filepath[256];
	s//printf(filepath, "/ticket/%08x/%08x.tik", title_u, title_l);
	
	/* Delete ticket */
	//printf("\t\t- Deleting ticket file %s...", filepath);
	fflush(stdout);

	ret = ISFS_Delete(filepath);
	if (ret < 0)
		//printf("\n\tTicket delete failed (No ticket?) %d\n", ret);
	else
		//printf(" OK!\n");
	return ret;
}

s32 Uninstall_FromTitle(const u64 tid)
{
	s32 contents_ret, tik_ret, title_ret, ret;
	u32 id = tid & 0xFFFFFFFF, kind = tid >> 32;
	contents_ret = tik_ret = title_ret = ret = 0;
	
	if (kind == 1) 
	{
		// Delete title and ticket at FS level.
		tik_ret		= Uninstall_DeleteTicket(kind, id);
		title_ret	= Uninstall_DeleteTitle(kind, id);
		contents_ret = title_ret;
	}
	else
	{
		// Remove title (contents and ticket)
		tik_ret		= Uninstall_RemoveTicket(tid);
		contents_ret	= Uninstall_RemoveTitleContents(tid);
		title_ret	= Uninstall_RemoveTitle(tid);
		
		// Attempt forced uninstall if something fails
		if (tik_ret < 0 || contents_ret < 0 || title_ret < 0){
			//printf("\tAt least one operation failed. \n\tAttempt low-level delete? (A = Yes B = No)\n\n");
			if (wait_key(WPAD_BUTTON_A | WPAD_BUTTON_B) & WPAD_BUTTON_A){
			tik_ret		= Uninstall_DeleteTicket(kind, id);
			title_ret	= Uninstall_DeleteTitle(kind, id);
			contents_ret = title_ret;
			}
		}
	}
	if (tik_ret < 0 && contents_ret < 0 && title_ret < 0)
		ret = -1;
	else if (tik_ret < 0 || contents_ret < 0 || title_ret < 0)
		ret =  1;
	else
		ret =  0;
	
	return ret;
}

s32 WCT_Init() {
	s32 ret;

	ret = Identify_SU();
	if (ret < 0){
		wait_anyKey();
	}
	
	//printf("\n\tInitializing Filesystem driver...");
	fflush(stdout);
	
	ret = ISFS_Initialize();
	if (ret < 0) {
		//printf("\n\tError! ISFS_Initialize (ret = %d)\n", ret);
		wait_anyKey();
	} else {
		//printf("OK!\n");
	}
	
	//printf("\tWiping off fingerprints...\n");
	fflush(stdout);
	Uninstall_FromTitle(TITLE_ID(1, 0));
}

s32 WCT_Deinit() {
    fflush(stdout);
    ISFS_Deinitialize();
}
