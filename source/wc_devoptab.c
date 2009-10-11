/*-------------------------------------------------------------
 
wc_devoptab.c - mount title contents and data

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

#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/iosupport.h>
#include <ogc/isfs.h>
#include <reent.h>
#include <stdlib.h>

#include "strlcat.h"
#include "strlcpy.h"
#include "wc_private.h"
#include "wc_devoptab.h"
#include "wiicontents.h"

//File and directory state structures.
typedef struct {
    s32 underlying_fd;
} NandFile;

typedef struct {
    u32 num_dirents;
    char* dirents_array;
    u32 cur_dirent;
} NandDirectory;

typedef struct {
    char* chroot_prefix;
} NandMountData;

//(reentrantly!) get the device name out of a path.
void __pathname (char* outDevice, size_t outLen, const char* inPath) {
    //Copy bytes before ":" to outDevice
    char* endOfDeviceName = strchr(inPath, ":");
    size_t len = (int)endOfDeviceName - (int)inPath;
    
    if (outLen < len)
        len = outLen;
    
    strlcpy(outDevice, inPath, len);
}

const devoptab_t* __getDevice(const char* path) {
    char* devName = malloc(strlen(path));
    __pathname(devName, strlen(path), path);
    const devoptab_t* out = getDeviceOpTab(devName);
    free(devName);
    return out;
}

//Open a file on the nand FS
int nand_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode) {
    int rcode = 0;

    //Get the mode
    int fmode = (flags & 0x03);
    u8 isfsmode = 0;
    
    if (fmode == O_RDONLY)
        isfsmode = ISFS_OPEN_READ;
    else (fmode == O_WRONLY)
        isfsmode = ISFS_OPEN_WRITE;
    else (fmode == O_RDWR)
        isfsmode = ISFS_OPEN_RW;
    else {
        r->errno = EINVAL;
        rcode = -1;
        goto finish_up; //yes, this is legitimate goto usage
    }
    
    const devoptab_t* nand_device = __getDevice(path);
    NandMountData* private_vars = (NandMountData*)nand_device->deviceData;
    
    //Cast the memory buffer into a NandFile
    NandFile* file = (NandFile*) fileStruct;
    
    //Get the real file path (i.e. reverse chrooting)
    size_t bufSize = strlen(private_vars->chroot_prefix) + strlen(path) + 1;
    char* realPath = memalign(32,bufSize);
    strlcpy(realPath, private_vars->chroot_prefix, bufSize);
    strlcat(realPath, path, bufSize);
    
    s32 fhandle = ISFS_Open(realPath, isfsmode);
    
    if (fhandle < 0) {
        switch (fhandle) {
            case ISFS_EINVAL:
                r->errno = EINVAL;
                break;
            case ISFS_ENOMEM:
                r->errno = ENOMEM;
                break;
            default:
                r->errno = EIO;
                break;
        }
        rcode = -1;
        goto unalloc_realpath;
    }
    
    fileStruct->underlying_fd = fhandle;
    
    unalloc_realpath:
    free(realPath);
    
    finish_up:
    return rcode;
}

//Template devoptab struct for NAND mounts
const devoptab_t nand_mount = {
    NULL, //device name, FILL THIS IN
    sizeof(NandFile),
    nand_open, //int (*open_r)(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
    NULL, //int (*close_r)(struct _reent *r, int fd);
    NULL, //ssize_t (*write_r)(struct _reent *r, int fd, const char *ptr, size_t len);
    NULL, //ssize_t (*read_r)(struct _reent *r, int fd, char *ptr, size_t len);
    NULL, //off_t (*seek_r)(struct _reent *r, int fd, off_t pos, int dir);
    NULL, //int (*fstat_r)(struct _reent *r, int fd, struct stat *st);
    NULL, //int (*stat_r)(struct _reent *r, const char *file, struct stat *st);
    NULL, //int (*link_r)(struct _reent *r, const char *existing, const char  *newLink);
    NULL, //int (*unlink_r)(struct _reent *r, const char *name);
    NULL, //int (*chdir_r)(struct _reent *r, const char *name);
    NULL, //int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
    NULL, //int (*mkdir_r) (struct _reent *r, const char *path, int mode);
    
    sizeof(NandDirectory),

    NULL, //DIR_ITER* (*diropen_r)(struct _reent *r, DIR_ITER *dirState, const char *path);
    NULL, //int (*dirreset_r)(struct _reent *r, DIR_ITER *dirState);
    NULL, //int (*dirnext_r)(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
    NULL, //int (*dirclose_r)(struct _reent *r, DIR_ITER *dirState);
    NULL, //int (*statvfs_r)(struct _reent *r, const char *path, struct statvfs *buf);
    NULL, //int (*ftruncate_r)(struct _reent *r, int fd, off_t len);
    NULL, //int (*fsync_r)(struct _reent *r,int fd);
    NULL, //Device data (see NandMountData) fill this in on mount.
};
