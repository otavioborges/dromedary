#ifndef SRC_SFTPFINCTIONS_H_
#define SRC_SFTPFINCTIONS_H_

#include "sftp.h"

namespace dromedary{
    class SftpFunctions{
        private:
            Sftp *m_sessionManager;
        public:
            SftpFunctions(Sftp *manager);
            bool Exists(char *path);
            void List(char *path);
    };
}

#endif // SRC_SFTPFINCTIONS_H_
