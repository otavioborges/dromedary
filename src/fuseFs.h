#ifndef SRC_FUSE_H_
#define SRC_FUSE_H_

#include <fuse.h>
#include "sftp.h"
#include "sftpFunctions.h"

namespace dromedary{
    class FuseFileSystem{
        private:
            Sftp *m_sftp;
            SftpFunctions *m_functions;
            char *m_mountPoint;
            struct fuse_chan *m_channel;
            struct fuse *m_session;
        public:
            FuseFileSystem(Sftp *sftp, SftpFunctions *functions);
            bool Mount(char *path);
            bool Unmount(void);
    };
}

#endif // SRC_FUSE_H_
