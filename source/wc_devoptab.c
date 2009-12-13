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
#include <ogc/ios.h> //These are needed for the error defines
#include <ogc/ipc.h> //This too
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
    //All prefixes must START with a / and must not END with a /.
    //If they are empty, they must be empty strings and not a single /.
    const char* chroot_prefix; //This is immovable.
    char* curdir_prefix; //This is movable.
} NandMountData;

typedef struct {
    PathList* prev;
    char* name;
    PathList* next;
} PathList;

const devoptab_t* __getDevice(const char* path) {
    char* devName = malloc(strlen(path));
    pathname(devName, strlen(path), path);
    const devoptab_t* out = getDeviceOpTab(devName);
    free(devName);
    return out;
}

//Emulate . and .. in paths by splitting a path into a linked list, removing the
//. and .. entries, and then collapsing it back into a Cstring.
void __collapsepath (char* outPath, size_t outLen, const char* inPath) {
    //Copy the const buffer into an internal buffer, since strtok writes NULLs
    char* pathbuf = strdup(inPath);
    
    //Setup the linkedlist
    PathList* root = malloc(sizeof(PathList));
    PathList* curPath = root;
    root->prev = NULL;
    root->name = NULL;
    root->next = NULL;
    
    char* curTok = strsep(&pathbuf, "/");
    
    //Build up the list
    do {
        if (*curTok == "." || *curTok == "\0") {
            continue;
        } else if (strcmp(curTok, "..") == 0) {
            //Are we at the root?
            if (curPath->prev = NULL)
                continue; //we can't go before the root
            else {
                //Remove the current directory
                curPath = curPath->prev;
                free(curPath->next);
                curPath->next = NULL;
            }
        } else {
            curPath->next = malloc(sizeof(PathList));
            curPath->name = curTok;
            curPath->next->prev = curPath;
            curPath->next->next = NULL; //always init your unused memory!
            
            curPath = curPath->next;
        }
        
        curTok = strsep(&pathbuf, "/");
    } while (curTok != NULL);
    
    //Collapse it back down
    *outPath = "\0" //Ensure the string is null terminated
    curPath = root;
    
    while (curPath != NULL && curPath->name != NULL) {
        strlcat(outPath, curPath->name, outLen);
        strlcat(outPath, "/", outLen);
        
        //Traverse backwards, deleting as we go
        curPath = curPath->prev;
        free(curPath->next);
    }
    
    free(pathbuf);
}

//Ensure that the path pointed to by inPath is a valid one.
int __validatepath (const char* inPath) {
    s32 ret = ISFS_ReadDir(inPath, NULL, NULL);
    
    if (ret == ISFS_OK) return TRUE;
    return FALSE;
}


//Take a relative or absolute path and write out a corrected path.
s32 __expandpath(size_t* correctLen, char *outPath, size_t pathLen, const char *inPath, NandMountData* private_vars) {
    s32 out = 0;
    size_t pathSize = strlen(private_vars->chroot_prefix) + strlen(inPath) + 1;
    char* tmpbuf = NULL;
    
    if (*inPath != "/") {
        //Non-absolute path
        pathSize += strlen(private_vars->curdir_prefix);
        
        if (outPath != NULL) {
            tmpbuf = malloc(pathSize);
            if (tmpbuf == NULL) {
                out = -1;
                goto error_nomem;
            }

            strlcpy(tmpbuf, private_vars->chroot_prefix, pathSize);            
            strlcat(tmpbuf, private_vars->curdir_prefix, pathSize);
            strlcat(tmpbuf, "/", pathSize);
            strlcat(tmpbuf, inPath, pathSize);
        
            __collapsepath(absOldPath, oldPathSize, absOldPath);
            strlcpy(outPath, tmpbuf, pathLen);
        }
    } else if (outPath != NULL) {
        //Absolute path
        tmpbuf = malloc(pathSize);
        if (tmpbuf == NULL) {
            out = -1;
            goto error_nomem;
        }

        strlcpy(tmpbuf, private_vars->chroot_prefix, pathSize);
        strlcat(tmpbuf, inPath, pathSize);

        __collapsepath(absOldPath, oldPathSize, absOldPath);
        strlcpy(outPath, tmpbuf, pathLen);
    }
    
    if (correctLen != NULL)
        *correctLen = pathSize;
        
    error_nomem:
    return;
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
        r->_errno = EINVAL;
        rcode = -1;
        goto finish_up; //yes, this is legitimate goto usage
    }
     
    const devoptab_t* nand_device = __getDevice(path);
    NandMountData* private_vars = (NandMountData*)nand_device->deviceData;
    
    //Cast the memory buffer into a NandFile
    NandFile* file = (NandFile*) fileStruct;
    
    //Strip out the device name
    char* dirs = strdup(path);
    pathdirs(dirs, strlen(path), path);
    
    size_t realPathSize;
    char* realPath = NULL;
    
    __expandpath(&realPathSize, realPath, 0, dirs, private_vars);
    realPath = malloc(realPathSize);
    
    if(realPath == NULL) {
        r->_errno = ENOMEM;
        rcode = -1;
        goto finish_up;
    }
    
    __expandpath(NULL, realPath, realPathSize, dirs, private_vars);
    
    s32 fhandle = ISFS_Open(realPath, isfsmode);
    
    if (fhandle < 0) {
        switch (fhandle) {
            case ISFS_EINVAL:
            case IPC_EINVAL:
                r->_errno = EINVAL;
                break;
            case ISFS_ENOMEM:
            case IPC_ENOMEM:
                r->_errno = ENOMEM;
                break;
            default:
                r->_errno = EIO;
                break;
        }
        rcode = -1;
        goto unalloc_realPath;
    }
    
    fileStruct->underlying_fd = fhandle;
    
    unalloc_realPath:
    free(realPath);
    
    unalloc_dirs:
    free(dirs);
    
    finish_up:
    return rcode;
}

int nand_close (struct _reent *r, int fd) {
    int out = 0;
    NandFile* fileStruct = (NandFile*) fd;

    if (fileStruct->underlying_fd < 0) {
        r->_errno = EBADF;
        out = -1;
        goto finish_up;
    }
    
    ISFS_Close(fileStruct->underlying_fd);

    fileStruct->underlying_fd = -1; //Invalidate the file.

    finish_up:
    return out;
}

ssize_t nand_write(struct _reent *r, int fd, const char *ptr, size_t len) {
    ssize_t out = 0;
    NandFile* fileStruct = (NandFile*) fd;

    //Sanity check tiem!
    if (fileStruct->underlying_fd < 0) {
        r->_errno = EBADF;
        out = -1;
        goto finish_up;
    }
        
    out = ISFS_Write(fileStruct->underlying_fd, ptr, len);
    if (out < 0) {
        switch (out) {
            case ISFS_EINVAL:
            case IPC_EINVAL:
                r->_errno = EINVAL;
                break;
            case ISFS_ENOMEM:
            case IPC_ENOMEM:
                r->_errno = ENOMEM;
                break;
            default:
                r->_errno = EIO;
        }
        out = -1;
        goto finish_up;
    }

    finish_up:
    return out;
}

ssize_t nand_read (struct _reent *r, int fd, char *ptr, size_t len) {
    ssize_t out = 0;
    NandFile* fileStruct = (NandFile*) fd;

    if (fileStruct->underlying_fd < 0) {
        r->_errno = EBADF;
        out = -1;
        goto finish_up;
    }

    out = ISFS_Read(fileStruct->underlying_fd, ptr, len);
    if (out < 0) {
        switch (out) {
            case ISFS_EINVAL:
            case IPC_EINVAL:
                r->_errno = EINVAL;
                break;
            case ISFS_ENOMEM:
            case IPC_ENOMEM:
                r->_errno = ENOMEM;
                break;
            default:
                r->_errno = EIO;
        }
        out = -1;
        goto finish_up;
    }

    finish_up:
    return out;
}

off_t nand_seek(struct _reent *r, int fd, off_t pos, int dir) {
    off_t out = 0;
    s32 ret = 0;
    NandFile* fileStruct = (NandFile*) fd;

    if (fileStruct->underlying_fd < 0) {
        r->_errno = EBADF;
        out = -1;
        goto finish_up;
    }

    ret = ISFS_Seek(fileStruct->underlying_fd, pos, dir);
    if (ret < 0) {
        switch (out) {
            case ISFS_EINVAL:
            case IPC_EINVAL:
                r->_errno = EINVAL;
                break;
            case ISFS_ENOMEM:
            case IPC_ENOMEM:
                r->_errno = ENOMEM;
                break;
            default:
                r->_errno = EIO;
        }
        out = -1;
        goto finish_up;
    } else {
        out = ret;
    }

    return out;
}

int nand_chdir (struct _reent *r, const char *name) {
    int out = 0;
    
    const devoptab_t* nand_device = __getDevice(name);
    NandMountData* private_vars = (NandMountData*)nand_device->deviceData;
    
    //Get the new cwd
    char* newPath = 0;
    size_t newPathSize = 0;
    
    if (*name = "/") { //simple case: Absolute chdir path replacement
        newPath = strdup(name);
    } else {
        newPathSize = strlen(private_vars->curdir_prefix) + strlen(name) + 2;
        newPath = malloc(newPathSize);
        
        strlcpy(newPath, private_vars->curdir_prefix, newPathSize);
        strlcat(newPath, "/", newPathSize);
        strlcat(newPath, name, newPathSize);
    }
    
    //Collapse the path
    __collapsepath(newPath, newPathSize, newPath);
    
    //Remove trailing slashes
    char* trailingslash = strrchr(newPath, "/");
    
    while (trailingslash != NULL) {
        *trailingslash = "\0";
        trailingslash = strrchr(newPath, "/");
    }
    
    //Check the path is vaild
    if (!__validatepath(newPath)) {
        free(newPath);
        out = -1;
        r->_errno = ENOTDIR;
        goto finish_up;
    }
    
    //Clean up
    set_curdir:
    free(private_vars->curdir_prefix);
    private_vars->curdir_prefix = newPath;
    
    finish_up:
    return out;
}

int nand_rename (struct _reent *r, const char *oldName, const char *newName) {
    //This is easy: the underlying function is s32 ISFS_Rename(const char *filepathOld,const char *filepathNew);
    //Convert 'em into absolute paths, un-chroot 'em, and call the underlying function.
    int out = 0;
    
    const devoptab_t* nand_device = __getDevice(path);
    NandMountData* private_vars = (NandMountData*)nand_device->deviceData;
    
    char* oldPath = strdup(oldName);
    if (oldPath == NULL) {
        out = -1;
        r->_errno = ENOMEM;
        goto finish_up;
    }
        
    char* newPath = strdup(newName);
    if (newPath == NULL) {
        out = -1;
        r->_errno = ENOMEM;
        goto dealloc_oldpath;
    }
    
    pathdirs(oldPath, strlen(oldName), oldName);
    pathdirs(newPath, strlen(newName), newName);
    
    //Now, to get absolute and correct paths...
    size_t oldPathSize;
    size_t newPathSize;
    
    __expandpath(&oldPathSize, NULL, 0, oldPath, private_vars);
    __expandpath(&newPathSize, NULL, 0, newPath, private_vars);
    
    char* oldPathAbs = malloc(oldPathSize);
    if (oldPathAbs == NULL) {
        out = -1;
        r->_errno = ENOMEM;
        goto dealloc_newpath;
    }
    
    char* newPathAbs = malloc(newPathSize);
    if (newPathAbs == NULL) {
        out = -1;
        r->_errno = ENOMEM;
        goto dealloc_oldpathabs;
    }
    
    __expandpath(NULL, oldPathAbs, oldPathSize, oldPath, private_vars);
    __expandpath(NULL, newPathAbs, newPathSize, newPath, private_vars);
    
    //Paths are correct, call ISFS
    s32 isfs_err = ISFS_Rename(oldPathAbs, newPathAbs);
    
    switch (isfs_err) {
        case ISFS_EINVAL:
        case IPC_EINVAL:
            r->_errno = EINVAL;
            out = -1;
            goto dealloc_newpathabs;
        case ISFS_ENOMEM:
        case IPC_ENOMEM:
            r->_errno = ENOMEM;
            out = -1;
            goto dealloc_newpathabs;
        default:
            if (isfs_err < 0) {
                r->_errno = EIO;
                out = -1;
                goto dealloc_newpathabs;
            }
    }
    
    dealloc_newpathabs:
    free(newPathAbs);
    
    dealloc_oldpathabs:
    free(oldPathAbs);
    
    dealloc_newpath:
    free(newPath);
    
    dealloc_oldpath:
    free(oldPath);
    
    finish_up:
    return out;
}

int nand_mkdir (struct _reent *r, const char *path, int mode) {
    //Not implemented: Need to find a way to auto-generate the user/group IDs.
}

//Template devoptab struct for NAND mounts
const devoptab_t nand_mount = {
    NULL, //device name, FILL THIS IN
    sizeof(NandFile),
    nand_open, //int (*open_r)(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
    nand_close, //int (*close_r)(struct _reent *r, int fd);
    nand_write, //ssize_t (*write_r)(struct _reent *r, int fd, const char *ptr, size_t len);
    nand_read, //ssize_t (*read_r)(struct _reent *r, int fd, char *ptr, size_t len);
    nand_seek, //off_t (*seek_r)(struct _reent *r, int fd, off_t pos, int dir);
    NULL, //int (*fstat_r)(struct _reent *r, int fd, struct stat *st);
    NULL, //int (*stat_r)(struct _reent *r, const char *file, struct stat *st);
    NULL, //int (*link_r)(struct _reent *r, const char *existing, const char  *newLink);
    NULL, //int (*unlink_r)(struct _reent *r, const char *name);
    nand_chdir, //int (*chdir_r)(struct _reent *r, const char *name);
    nand_rename, //int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
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
