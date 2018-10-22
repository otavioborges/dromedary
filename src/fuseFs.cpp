#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include "fuseFs.h"
using namespace dromedary;

static int fuseGetAttr(const char *path, struct stat *stbuf){
    int response = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0){
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }else if(strcmp(path+1, "hello") == 0){
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 6;
    }else{
        response = -ENOENT;
    }

    return response;
}

static int fuseReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    (void) offset;
    (void) fi;

    if(strcmp(path, "/") != 0)
        return -ENOENT;
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "hello", NULL, 0);

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

FuseFileSystem::FuseFileSystem(Sftp *sftp, SftpFunctions *functions){
    m_sftp = sftp;
    m_functions = functions;
}
bool FuseFileSystem::Mount(char *path){
    m_mountPoint = new char[strlen(path)];
    strcpy(m_mountPoint, path);

    struct fuse_operations oper;
    oper.getattr = fuseGetAttr;
    oper.readdir = fuseReadDir;
    oper.open    = fuseOpen;
    oper.read    = fuseRead;

    m_channel = fuse_mount(m_mountPoint, NULL);
    m_session = fuse_new(m_channel, NULL, &oper, sizeof(oper), NULL);
    fuse_loop(m_session);

    return true;
}

bool FuseFileSystem::Unmount(void){
    fuse_destroy(m_session);
    fuse_unmount(m_mountPoint, m_channel);

    return true;
}
