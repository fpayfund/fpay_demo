#include <iostream>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sstream>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string>
#include "Socket.h"
#include "logging.h"

using namespace std;

    template < class T1 >
inline std::string tostring(const T1 & t1)
{
    std::ostringstream ss;
    ss << t1;
    return ss.str();
}

    template < class T1, class T2 >
inline std::string tostring(const T1 & t1, const T2 & t2)
{
    std::ostringstream ss;
    ss << t1 << t2;
    return ss.str();
}

inline int socketExn::getLastError()
{
    return errno;
}

inline bool Socket::isIgnoreAcceptError(int en) const
{
    return en == EWOULDBLOCK || en == EINTR || en == ECONNABORTED || en == EMFILE;
}

inline bool Socket::isIgnoreError(int en) const
{
    return en == EAGAIN || en == EINTR || EINPROGRESS == en;
}

inline bool Socket::isIgnoreConnect(int en) const
{
    return EWOULDBLOCK == en || EINPROGRESS == en;
}

void Socket::setblocking(bool blocking)
{
    int fflags = ::fcntl(m_socket, F_GETFL);
    if (-1 == fflags)
        throw socketExn("setblocking F_GETFL");

    if (blocking) {
        fflags &= ~O_NONBLOCK;
    } else {
        fflags |= O_NONBLOCK;
    }

    if (-1 == ::fcntl(m_socket, F_SETFL, fflags))
        throw socketExn("setblocking F_SETFL");

    m_sock_flags.nonblocking = (blocking ? 0 : 1);
}

void Socket::setCloseExec()
{
    int fflags = ::fcntl(m_socket, F_SETFD, FD_CLOEXEC);
    if (-1 == fflags)
        throw socketExn("setblocking F_SETFD close exec");
    m_sock_flags.closeExe = 1;
}

inline int Socket::how_shutdown(int nHow)
{
    switch (nHow)
    {
    case Receives :
        return SHUT_RD;
    case Sends :
        return SHUT_WR;
    default: // Both :
        return SHUT_RDWR;
    }
}

int Socket::recv(void * lpBuf, size_t nBufLen, int flag)
{
    m_sock_flags.recv_tag = 1;
    setRecvTime();
    int ret = ::recv(m_socket, (char*)lpBuf, nBufLen, flag);
    if (-1 == ret) {
        // XXX howto ignore EAGAIN and EINTR
        throw socketExn("recv");
    }
    return ret;
}

int Socket::send(const void * lpBuf, size_t nBufLen, int flag)
{
    m_sock_flags.send_tag = 1;

    int ret = ::send(m_socket, (const char*)lpBuf, nBufLen, flag);

    if (-1 == ret) {
        int en = socketExn::getLastError();
        if (isIgnoreError(en))
            return 0;
        throw socketExn(en, "send");
    }

    return ret;
}

int Socket::sendto(const void * msg, size_t len, u_long ip, int port)
{
    ipaddr_type sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = ip;
    sa.sin_port = htons((u_short)port);

    return sendto(msg, len, &sa, sizeof(sa));
}

int Socket::sendto(const void * msg, size_t len, const ipaddr_type *to, socklen_t tolen)
{
    m_sock_flags.send_tag = 1;
    int ret = ::sendto(m_socket, (const char *)msg, len, 0, (struct sockaddr*)to, tolen);

    if (-1 == ret) {
        int en = socketExn::getLastError();
        if (isIgnoreError(en)) return 0;
        throw socketExn(en, "sendto");
    }

    return ret;
}

int Socket::recvfrom(void * buf, size_t len, ipaddr_type *from, socklen_t *fromlen)
{
    m_sock_flags.recv_tag = 1;
    setRecvTime();
    int ret = ::recvfrom(m_socket, (char *)buf, len, 0, (struct sockaddr*)from, fromlen);

    if (-1 == ret) {
        int en = socketExn::getLastError();
        if (isIgnoreError(en)) {
            // igore again ant interrupt
            return 0;
        }
        throw socketExn(en, "recvfrom");
    }

    return ret;
}

void Socket::socket(int type, int domain)
{
    m_socket = ::socket(domain, type, 0);

    if (-1 == m_socket)
        throw socketExn("create socket");

#ifndef WIN32
    else
        fcntl(m_socket, F_SETFD, fcntl(m_socket, F_GETFD) | FD_CLOEXEC);
#endif
}

int Socket::detach()
{
      int so = m_socket;
      m_socket = INVALID_SOCK;
      m_sock_flags.reset();
      return so;
}
  
void Socket::shutdown(int nHow)
{
    if (nHow & Receives) {
        if (m_sock_flags.selevent & EVENT_READ)
            throw socketExn(0, "shutdown recv, but SEL_READ setup");
        m_sock_flags.shutdown_recv = 1;
    }
    if (nHow & Sends) {
        if (m_sock_flags.selevent & EVENT_WRITE)
            throw socketExn(0, "shutdown send, but SEL_WRITE setup");
        m_sock_flags.shutdown_send = 1;
    }
    ::shutdown(m_socket, how_shutdown(nHow));
}

void Socket::Close()
{
    if (isValid()) {
        close(m_socket);
        m_sock_flags.reset();
        m_socket = INVALID_SOCK;
    }
}

void Socket::setnodelay()
{
    int op = 1;
    if (!setsockopt(IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op)))
        throw socketExn("setnodelay");
}   

void Socket::setreuse()
{
    int op = 1;
    if (!setsockopt(SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)))
        throw socketExn("setreuse");
}     

int  Socket::getsndbuf() const
{
    int size = 0;
    socklen_t len = sizeof(size);
    if (!getsockopt(SOL_SOCKET, SO_SNDBUF, &size, &len))
        throw socketExn("getsndbuf");
    return size;
}

int  Socket::getrcvbuf() const
{
    int size = 0;
    socklen_t len = sizeof(size);
    if (!getsockopt(SOL_SOCKET, SO_RCVBUF, &size, &len))
        throw socketExn("getrcvbuf");
    return size;
}

void Socket::setsndbuf(int size)
{
    if (!setsockopt(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)))
        throw socketExn("setsndbuf");
}

void Socket::setrcvbuf(int size)
{
    if (!setsockopt(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)))
        throw socketExn("setrcvbuf");
}

void Socket::listen()
{
    if (-1 == ::listen(m_socket, SOMAXCONN))
        throw socketExn("listen");
    m_sock_flags.tcpserver = 1;
}

void Socket::bind(const std::string & ip, int port)
{
    u_long addr = INADDR_ANY;
    if (!ip.empty()) {
        addr = ::inet_addr(ip.c_str());
        if (addr == INADDR_NONE)
            throw socketExn(0, tostring("bind: invalid ip=", ip));
    }

    bind(addr, port);
}

int Socket::accept(std::string* ip, int* port)
{
    u_long addr;
    int p;

    int newsock = accept(&addr, &p);

    if (ip) {
        *ip = addr_ntoa(addr);
    }
    if (port) {
        *port = p;
    }

    return newsock;
}

int Socket::accept(u_long * addr, int * port)
{
    ipaddr_type sa;
    socklen_t len = sizeof(sa);

    int ret = ::accept(m_socket, (struct sockaddr*)&sa, &len);
    if (-1 == ret) {
        int en = socketExn::getLastError();
        if (isIgnoreAcceptError(en))
            return -1;
        throw socketExn(en, "accept");
    }

    if (addr) *addr = sa.sin_addr.s_addr;
    if (port) *port = ntohs(sa.sin_port);

    return ret;
}

void Socket::bind(int port, u_long addr)
{
    ipaddr_type sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr;
    sa.sin_port = htons((u_short)port);

    if (-1 == ::bind(m_socket, (struct sockaddr*)&sa, sizeof(sa)))
        throw socketExn(tostring("bind at ", port));
}

void Socket::bind(const char * lpszip, int port)
{
    u_long addr = INADDR_ANY;
    if (lpszip && strlen(lpszip) > 0) {
        addr = ::inet_addr(lpszip);
        if (addr == INADDR_NONE)
            throw socketExn(0, tostring("bind: invalid ip=", lpszip));
    }
    bind(port, addr);
}

u_long Socket::getpeer(int *port) const
{
    ipaddr_type sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t len = sizeof(sa);

    if (-1 == ::getpeername(m_socket, (struct sockaddr *)&sa, &len))
        throw socketExn("getpeer");

    if (port) *port = ntohs(sa.sin_port);
    return sa.sin_addr.s_addr;
}

u_long Socket::getlocal(int *port) const
{
    ipaddr_type sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t len = sizeof(sa);

    if (-1 == ::getsockname(m_socket, (struct sockaddr*)&sa, &len))
        throw socketExn("getlocal");

    if (port) *port = ntohs(sa.sin_port);
    return sa.sin_addr.s_addr;
}

bool Socket::connect(u_long ip, int port)
{
    ipaddr_type sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = ip;
    sa.sin_port = htons((u_short)port);

    if (-1 == ::connect(m_socket, (struct sockaddr*)&sa, sizeof(sa))) {
        int en = socketExn::getLastError();
        LOG_INFO <<"error socket port:" << port << ", error no is:" << en;
        if (isIgnoreConnect(en))
            return false;
        throw socketExn(en, "connect");
    }

    {
        u_long localip; int localport;
        localip = getlocal(&localport);
        if (localip == ip && localport == port)
            throw socketExn(0, "dereism connection found");
    }

    m_sock_flags.connected = 1;

    return true;
}

std::string Socket::complete_nonblocking_connect()
{
    int e = 0;
    socklen_t l = sizeof(e);

    if (!getsockopt(SOL_SOCKET, SO_ERROR, &e, &l))
        e = socketExn::getLastError();
     
    if (e) {
        socketExn err(e, "nonblock connect");
        return err.what_str();
    }

    m_sock_flags.connected = 1;

    return std::string("connection ok");
}

void Socket::setRecvTime(uint32_t now)
{           
    m_lastRcvTime = now;
}

uint32_t Socket::getRecvTime()
{
    return m_lastRcvTime;
}

