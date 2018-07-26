#ifndef __CONN_MANAGER_H__
#define __CONN_MANAGER_H__
#include <set>
#include "base.h"
#include "EventHandler.h"

class ConnManager
    : public IConnManager
    , public IConnEventHandler
{
public:
    virtual void onConnected(IConn *conn);
    virtual void onClose(IConn *conn);
    virtual void onInitiativeClose(IConn *conn);
    virtual void onError(int ev, const char *msg, IConn *conn);
    virtual void onTimeout(IConn *conn);
};  

class LazyDelConnManager
    : public ConnManager
    , public IDelayDelConn
    , public EventHandler
{
public:
    LazyDelConnManager();

    virtual void doEvent(int event);
    virtual void destroy() { }
    virtual void DelayDelConn(IConn *conn);

    IConn *createServerSideConn(int fd, uint32_t ip, int port,
            IProtoConsumer *consumer, IConnEventHandler *handler);
    IConn *createClientSideConn(const std::string& ip, uint32_t port,
            IProtoConsumer *consumer, IConnEventHandler *handler);

protected:
    std::set<uint32_t> lazyDelIds;
};

#endif
