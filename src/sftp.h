#ifndef SRC_SFTP_H_
#define SRC_SFTP_H_

#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace dromedary{
    class Sftp{
        private:
            char *m_server;
            int m_port;
            ssh_key m_key;
            ssh_session m_sshSession;
            sftp_session m_sftpSession;
            bool m_isConnected;
        public:
            Sftp(char *server, int port);
            bool LoadKey(char *path);
            bool Connect(char * username);
            void Disconnect(void);
            bool isConnected(void);

            // GETs
            ssh_session *GetSSHSession(void);
            sftp_session *GetSFTPSession(void);
    };
}

#endif // SRC_SFTP_H_