#ifndef SRC_SFTPFINCTIONS_H_
#define SRC_SFTPFINCTIONS_H_

#include "sftp.h"

namespace dromedary{
    class SftpFunctions{
        private:
            Sftp *m_sessionManager;
        public:
            SftpFunctions(Sftp *manager);
            void List(char *path);
    };
}

#endif // SRC_SFTPFINCTIONS_H_
