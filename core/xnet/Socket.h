#ifndef _SOCKET_HELPER_H_
#define _SOCKET_HELPER_H_
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include "typedef.h"
#include "Exception.h"

enum {
    EVENT_NONE = 0, 
    EVENT_ALL = -1,
    EVENT_READ = 1,
    EVENT_WRITE = 2, 
    EVENT_RW = 3,
    EVENT_TIMEOUT = 8,
    EVENT_CONNECTING = 128,
};

class Socket;

class socketExn : public Exception
{   
public:
    static int getLastError();

    socketExn(const std::string& what_arg)
        : Exception(getLastError(), what_arg)
    {}
private:
    friend class Socket;
    socketExn(int e, const std::string& what_arg)
        : Exception(e, what_arg)
    {}   
};

inline bool valid_addr(uint32_t ip)
{
    return ip != INADDR_NONE;
}
inline u_long aton_addr(const char* ip)
{
    return ::inet_addr(ip); 
}
inline u_long aton_addr(const std::string& ip)
{
    return aton_addr(ip.c_str());
}

// XXX thread ?
inline std::string addr_ntoa(uint32_t ip)
{
    struct in_addr addr;
    memcpy(&addr, &ip, 4);
    return std::string(::inet_ntoa(addr));
}

typedef struct sockaddr_in ipaddr_type;

class Socket
{
public:
    Socket()
        : m_socket(INVALID_SOCK)
    {}
    virtual ~Socket()
    {
        Close();
    }
    int getsocket() const
    {
        return m_socket;
    }
    int getsocket()
    {
        return m_socket;
    }
    void attach(int so)
    {
        assert(!isValid());
        m_socket = so;
        m_sock_flags.connected = 1;
    }
    bool isValid() const
    {
        return (m_socket != INVALID_SOCK);
    }
    bool isBlocking() const
    {
        return m_sock_flags.nonblocking == 0; 
    }
    bool isConnected() const
    {
        return m_sock_flags.connected == 1;
    }

    enum { Receives = 1, Sends = 2, Both = 3 };

    void shutdown(int nHow = Sends);
    bool isShutdownSends() const
    {
        return m_sock_flags.shutdown_send == 1;
    }
    bool isShutdownReceives() const
    {
        return m_sock_flags.shutdown_recv == 1;
    }

    bool checkSendTag() const
    {
        return m_sock_flags.send_tag == 1;
    }
    bool checkRecvTag() const
    {
        return m_sock_flags.recv_tag == 1;
    }
    // clear and return old tag
    bool clearSendTag()
    {
        bool b = checkSendTag();
        m_sock_flags.send_tag = 0;
        return b;
    }
    bool clearRecvTag()
    {
        bool b = checkRecvTag();
        m_sock_flags.recv_tag = 0;
        return b;
    }

    void setRecvTag()
    {
        m_sock_flags.recv_tag = 1;
    }
    void setRecvTime(uint32_t now=0);
    uint32_t getRecvTime();

    // create
    void socket(int type = SOCK_STREAM, int domain = AF_INET);

    void setnodelay();
    void setreuse();
    void setblocking(bool blocking);
    void setCloseExec();
    void setsndbuf(int size);
    void setrcvbuf(int size);
    int  getsndbuf() const;
    int  getrcvbuf() const;

    std::string getpeerip(int* port = NULL) const
    {
        return addr_ntoa(getpeer(port));
    }
    std::string getlocalip(int* port = NULL) const
    {
        return addr_ntoa(getlocal(port));
    }
    u_long getpeer(int * port = NULL) const;
    u_long getlocal(int * port = NULL) const;

    // >0 : bytes recv
    // 0  : peer close normal
    // <0 : error (reserve)
    //    : throw socket-error
    int recv(void * buf, size_t nBufLen, int flag=0);
    // >=0 : bytes send;
    // <0  : error (reserve)
    //     : throw socketExn
    int send(const void * buf, size_t nBufLen, int flag=0);

    // >0 : bytes sendto
    // 0  : isOk or isIgnoreError
    // <0 : error (reserve)
    //    : throw socketExn
    int sendto(const void * msg, size_t len, const ipaddr_type *to, socklen_t tolen);
    int sendto(const void * msg, size_t len, u_long ip, int port);
    int sendto(const void * msg, size_t len, const std::string &ip, int port)
    {
        return sendto(msg, len, aton_addr(ip), port);
    }
    // >0 : bytes recvfrom
    // 0  : isOk or isIgnoreError
    // <0 : error (reserve)
    //    : throw socketExn
    int recvfrom(void * buf, size_t len, ipaddr_type *from, socklen_t *fromlen);

    // true  : connect completed
    // false : nonblocking-connect inprocess
    //       : throw socketExn
    bool connect(const std::string & ip, int port)
    {
        return connect(aton_addr(ip), port);
    }
    bool connect(u_long ip, int port);
    std::string complete_nonblocking_connect();

    // >=0 : accepted socket
    // <0  : some error can ignore. for accept more than once
    //     : throe socketExn
    int accept(u_long * addr = NULL, int * port = NULL);
    int accept(std::string * ip, int * port = NULL);

    void bind(const std::string & ip, int port);
    void bind(const char *ip, int port);
    void bind(int port, u_long addr = INADDR_ANY);
    void listen();

    static int soclose(int so)
    {
        return close(so);
    }

    static int try_bind(const char* ip, unsigned short & port);

    int detach();

protected:
    bool getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
    {
        return (-1 != ::getsockopt(m_socket, level, optname, (char*)optval, optlen));
    }
    bool setsockopt(int level, int optname, const void *optval, socklen_t optlen)
    {
        return (-1 != ::setsockopt(m_socket, level, optname, (const char *)optval, optlen));
    }

    int how_shutdown(int nHow);
    bool isIgnoreConnect(int en) const;
    bool isIgnoreError(int en) const;
    bool isIgnoreAcceptError(int en) const;
public:
    struct SockFlags {
        // used by Socket
        unsigned int selevent : 8;
        unsigned int selected : 1;
        // inner
        unsigned int connected     : 1;
        unsigned int nonblocking   : 1;
        unsigned int closeExe      : 1; 
        unsigned int tcpserver     : 1;
        unsigned int shutdown_send : 1;
        unsigned int shutdown_recv : 1;
        unsigned int send_tag : 1;
        unsigned int recv_tag : 1;

        SockFlags()
        {
            reset();
        }
        /* XXX clear all */
        void reset()
        {
            *((unsigned int*)this) = 0;
        }
    } m_sock_flags;

protected:
    friend class SocketHandler;
    void Close(); // careful if it's selected. hide now

private:
    uint32_t m_lastRcvTime;
    int m_socket;

};

#endif
