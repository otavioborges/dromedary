#ifndef SRC_FUSE_H_
#define SRC_FUSE_H_

#define FUSE_USE_VERSION 26

#include <pthread.h>
#include <fuse.h>
#include "sftp.h"

void InitFuse(dromedary::Sftp *manager);
bool Mount(char *mountpoint);
void Unmount(void);

#endif // SRC_FUSE_H_
