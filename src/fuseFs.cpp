#define FUSE_LOG_ID     "FUSE"

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <libssh/sftp.h>
#include "fuseFs.h"
#include "LogInfo.h"
using namespace dromedary;

typedef struct openedFile{
    char *path;
    sftp_file file;
    bool opened;
    uint64_t length;
};

openedFile currentFile[8];
int filesCount = 0;

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
static int fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int fuseMkdir(const char *path, mode_t mode);
static int fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fi);
static int fuseUnlink(const char *path);
static int fuseChown(const char *path, uid_t uid, gid_t gid);
static int fuseChmod(const char *path, mode_t mode);
static int fuseRename(const char *original, const char *newname);

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
    operations.mkdir   = fuseMkdir;
    operations.open    = fuseOpen;
    operations.read    = fuseRead;
    operations.write   = fuseWrite;
    operations.create  = fuseCreate;
    operations.readdir = fuseReadDir;
    operations.unlink  = fuseUnlink;
    operations.chown   = fuseChown;
    operations.chmod   = fuseChmod;
    operations.rename  = fuseRename;
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
    fuse_unmount(gMountPoint, gChannel);
    pthread_join(gFuseThread, NULL);

    fuse_destroy(gFuse);
}

static int fuseGetAttr(const char *path, struct stat *stbuf){
    int response = 0;
    sftp_attributes at = sftp_stat(*gManager->GetSFTPSession(), path);
    if(at == NULL){
        response = -ENOENT;
    }else{
        memset(stbuf, 0, sizeof(struct stat));
        stbuf->st_mode  = (at->type << 16) | at->permissions;
        stbuf->st_nlink = 1;
        stbuf->st_size  = at->size;
        stbuf->st_gid   = at->gid;
        stbuf->st_uid   = at->uid;
        stbuf->st_size  = at->size;
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
    if(filesCount >= 8)
        return -EMFILE;

    sftp_file file = sftp_open(*gManager->GetSFTPSession(), path, fi->flags, 0);
    if(file == NULL)
        return -ENOENT;

    sftp_attributes attr = sftp_stat(*gManager->GetSFTPSession(), path);
    if(attr == NULL)
        return -ENOENT;

    currentFile[filesCount].path = new char[strlen(path)];
    strcpy(currentFile[filesCount].path, path);
    currentFile[filesCount].file = file;
    currentFile[filesCount].opened = true;
    currentFile[filesCount].length = attr->size;
    filesCount++;

    return 0;
}

static int fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    (void) fi;
    int fileIdx;

    for(fileIdx = 0; fileIdx < filesCount; fileIdx++){
        if(strcmp(path, currentFile[fileIdx].path) == 0)
            break;
    }

    if(fileIdx >= filesCount){
        LogInfo::AddEntry(FUSE_LOG_ID, "Trying to read on closed file, ignore", LogLevel::WARNING);
        return -EBADF;
    }

    if(offset > currentFile[fileIdx].length){
        return 0; // beyond file size
    }else{
        if((size + offset) > currentFile[fileIdx].length)
            size = currentFile[fileIdx].length - offset;
        
        if(currentFile[fileIdx].file->offset != offset)
            sftp_seek(currentFile[fileIdx].file, offset);
        
        return sftp_read(currentFile[fileIdx].file, buf, size);
    }
}

static int fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int fileIdx;

    for(fileIdx = 0; fileIdx < filesCount; fileIdx++){
        if(strcmp(path, currentFile[fileIdx].path) == 0)
            break;
    }

    if(fileIdx >= filesCount){
        LogInfo::AddEntry(FUSE_LOG_ID, "Trying to read on closed file, ignore", LogLevel::WARNING);
        return -EBADF;
    }

    if(currentFile[fileIdx].file->offset != offset)
        sftp_seek(currentFile[fileIdx].file, offset);
    
    return sftp_write(currentFile[fileIdx].file, buf, size);
}

static int fuseMkdir(const char *path, mode_t mode){
    mode = mode | S_IFDIR; // set correct mode bits

    return sftp_mkdir(*gManager->GetSFTPSession(), path, mode);
}

static int fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fi){
    sftp_file file = sftp_open(*gManager->GetSFTPSession(), path, fi->flags, mode); // always allow 
    if(file == NULL)
        return -ENOENT;
    
    if(sftp_close(file) != 0)
        return -EACCES;
    
    return sftp_chown(*gManager->GetSFTPSession(), path, 1000, 1000); // default to user on config
}

static int fuseUnlink(const char *path){
    return sftp_unlink(*gManager->GetSFTPSession(), path);
}

static int fuseChown(const char *path, uid_t uid, gid_t gid){
    return sftp_chown(*gManager->GetSFTPSession(), path, uid, gid);
}

static int fuseChmod(const char *path, mode_t mode){
    return sftp_chmod(*gManager->GetSFTPSession(), path, mode);
}

static int fuseRename(const char *original, const char *newname){
    return sftp_rename(*gManager->GetSFTPSession(), original, newname);
}

static int fuseRelease(const char *path, struct fuse_file_info *fi){

}
