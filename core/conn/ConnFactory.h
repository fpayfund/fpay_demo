#ifndef __CONN_FACTORY__
#define __CONN_FACTORY__

#include "base.h"
#include "Socket.h"
#include "ProtoConsumer.h"

class ConnFactory
    : public ServerConnFactory
    , public ClientConnFactory
{
public:
    ConnFactory()
        : m_bTcpNodelay(true)
    {}
    IConn *createConnection(const std::string& ip, uint32_t port,
        IProtoConsumer* consumer, IConnEventHandler* handler, CreateCallback* callback); 
    virtual IConn *createConnection(int fd, uint32_t ip, int port,
        IProtoConsumer* consumer, IConnEventHandler *handler, CreateCallback* callback); 
    void setConnTcpNodelay(bool flag)
    {
        m_bTcpNodelay = flag;
    } 

protected:
    bool m_bTcpNodelay;
};

template<class C>
class ServerSideConnFactory : public ServerConnFactory
{
public:
    ServerSideConnFactory ()
    {}
    virtual IConn* createConnection(int fd, uint32_t ip, int port,
            IProtoConsumer* consumer, IConnEventHandler* handler, CreateCallback* callback) 
    {
        C *conn = new C(fd, ip, port, consumer, handler, true);
        conn->setProtoConsumer(consumer);
        conn->setConnEventHandler(handler);
        if(callback) {
            callback->onConnCreate(conn);
        }
        conn->addEvent(0, EVENT_READ);
        return conn;
    }   
};  

#endif
