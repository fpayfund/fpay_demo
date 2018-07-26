#ifndef __TCP_ACCEPTOR_H__
#define __TCP_ACCEPTOR_H__

#include "EventHandler.h"
#include "typedef.h"

class WhiteLst;

enum {
    SOCKOPT_REUSE    = 1,
    SOCKOPT_NONBLOCK = 2,
    SOCKOPT_NODELAY  = 4,
    SOCKOPT_LISTEN   = 8,
    SOCKOPT_DEFAULT  = (SOCKOPT_REUSE | SOCKOPT_NONBLOCK | SOCKOPT_NODELAY | SOCKOPT_LISTEN),
};

class TcpAcceptor: public SocketHandler
{
public:
    TcpAcceptor(int binded, unsigned int ops = SOCKOPT_DEFAULT);
    TcpAcceptor(const char * ip, int port, unsigned int ops = SOCKOPT_DEFAULT);

    unsigned int get_ops() const
    {
        return m_ops;
    }
    WhiteLst * get_hosts_allow()
    {
        return m_hosts_allow;
    }
    void set_hosts_allow(WhiteLst * hosts_allow)
    {
        m_hosts_allow = hosts_allow;
    }

protected:
    virtual void onAccept(int so, uint32_t ip, int port) = 0;
    virtual void doEvent(int ev);
    virtual void onTimeout();

    WhiteLst * m_hosts_allow;
    unsigned int m_ops;
};

#endif
