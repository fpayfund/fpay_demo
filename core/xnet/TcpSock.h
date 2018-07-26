#ifndef __TCP_SOCK_H__
#define __TCP_SOCK_H__

#include "EventHandler.h"

class TcpSock : public SocketHandler
{
public:
    TcpSock()
    {} 
    TcpSock(int sock)
    {
        doAccept(sock);
    }
    TcpSock(u_long ip, int port, int timeout)
    {
        doConnect(ip, port, timeout);
    }
    TcpSock(const std::string& ip, int port, int timeout)
    {
        doConnect(ip, port, timeout);
    }

    void doAccept(int sock)
    { 
        socket().attach(sock);
        socket().setblocking(false);
    }   
    void doConnect(const std::string& ip, int port, int timeout)
    {
        doConnect(aton_addr(ip), port, timeout);
    }
    void doConnect(u_long ip, int port, int timeout)
    {   
        socket().socket();
        if (timeout > 0) {
            socket().setblocking(false);
        }
        if (!socket().connect(ip, port)) {
            addTimer(timeout);
        }
    }   

    virtual void addEvent(int remove, int add);
    virtual ~TcpSock();

protected:
    virtual void doEvent(int event);

    virtual void onRead() = 0;
    virtual void onWrite() = 0;
    virtual void onTimeout();
    virtual void onConnected(const std::string & errstr);
};

#endif
