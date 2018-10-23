#define FUSE_LOG_ID     "FUSE"

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <libssh/sftp.h>
#include "fuseFs.h"
#include "LogInfo.h"
using namespace dromedary;

static void *FuseThread(void *arg){
    if(arg == NULL){
        return NULL;
    }

    struct fuse *fs = (struct fuse *)arg;
    if (fuse_loop(fs) < 0) {
        // signal error
        return NULL;
    }

    return NULL;
}

static int fuseGetAttr(const char *path, struct stat *stbuf);
static int fuseReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int fuseOpen(const char *path, struct fuse_file_info *fi);
static int fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

Sftp                *gManager = NULL;
char                *gMountPoint = NULL;
pthread_t           gFuseThread;

struct fuse_chan    *gChannel;
struct fuse         *gFuse;
static struct fuse_operations operations;

void InitFuse(Sftp *manager){
    LogInfo::AddEntry(FUSE_LOG_ID, "Setting default SFTP manager", LogLevel::DEBUG);
    gManager = manager;

    operations.getattr = fuseGetAttr;
    operations.open    = fuseOpen;
    operations.read    = fuseRead;
    operations.readdir = fuseReadDir;
}

bool Mount(char *mountpoint){
    if(gManager == NULL){
        LogInfo::AddEntry(FUSE_LOG_ID, "Cannot mount without a defined SFTP manager", LogLevel::ERROR);
        return false;
    }

    gMountPoint = new char[strlen(mountpoint)];
    strcpy(gMountPoint, mountpoint);

    gChannel = fuse_mount(gMountPoint, NULL);
    gFuse = fuse_new(gChannel, NULL, &operations, sizeof(operations), NULL);
    pthread_create(&gFuseThread, NULL, FuseThread, (void *)gFuse);

    return true;
}

void Unmount(void){
    fuse_destroy(gFuse);
    fuse_unmount(gMountPoint, gChannel);
}

static int fuseGetAttr(const char *path, struct stat *stbuf){
    int response = 0;
    sftp_attributes at = sftp_stat(*gManager->GetSFTPSession(), path);
    if(at == NULL){
        response = -ENOENT;
    }else{
        memset(stbuf, 0, sizeof(struct stat));
        stbuf->st_mode = (at->type << 16) | at->permissions;
        stbuf->st_nlink = 1;
        stbuf->st_size = at->size;
    }
    
    // memset(stbuf, 0, sizeof(struct stat));
    // if(strcmp(path, "/") == 0){
    //     stbuf->st_mode = S_IFDIR | 0755;
    //     stbuf->st_nlink = 2;
    // }else if(strcmp(path+1, "hello") == 0){
    //     stbuf->st_mode = S_IFREG | 0444;
    //     stbuf->st_nlink = 1;
    //     stbuf->st_size = 6;
    // }else{
    //     response = -ENOENT;
    // }

    return response;
}

static int fuseReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    (void) offset;
    (void) fi;
    sftp_dir dir;
    sftp_attributes attibutes;

    dir = sftp_opendir(*gManager->GetSFTPSession(), path);
    if(!dir)
        return -ENOENT;

    while((attibutes = sftp_readdir(*gManager->GetSFTPSession(), dir)) != NULL){
        filler(buf, attibutes->name, NULL, 0);
    }

    return 0;
}

static int fuseOpen(const char *path, struct fuse_file_info *fi){
    if(strcmp(path+1, "hello") != 0)
        return -ENOENT;
    
    if((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    size_t len;
    (void) fi;

    if(strcmp(path+1, "hello") != 0)
        return -ENOENT;
    
    len = 6;
    if(offset < len){
        if(offset + size > len)
            size = len - offset;
        
        memcpy(buf, "hello\n" + offset, size);
    }else{
        size = 0;
    }

    return size;
}
