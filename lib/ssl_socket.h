#ifdef LIB_SLL_SOCKET_H
#define LIB_SSL_SOCKET_H

namespace dromedary {
  class ssl_socket {
    private:
      ssh_session m_session;
      ssh_bind    m_sshBind;

    public:
      ssl_socket(int port);
  }
}

#endif // LIB_SSL_SOCKET_H
