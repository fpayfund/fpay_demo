#ifndef __CONN_EVENT_HANDLER_H__
#define __CONN_EVENT_HANDLER_H__

class IConn;

class IConnEventHandler
{
public:
   virtual ~IConnEventHandler() {}
   virtual void onConnected(IConn *conn) = 0;
   virtual void onClose(IConn *conn) = 0;
   //主动关闭
   virtual void onInitiativeClose(IConn *conn) = 0;
   virtual void onError(int ev, const char *msg, IConn *conn) = 0;
   virtual void onTimeout(IConn *conn) = 0;
   virtual void updateRecvTime() {}
};

class IConnEventHandlerAware
{
public:
    IConnEventHandler *_eventHandler;
    
    IConnEventHandlerAware()
        : _eventHandler(NULL)
    {}
    virtual ~IConnEventHandlerAware() {}
    virtual void setConnEventHandler(IConnEventHandler *h)
    {
        _eventHandler= h;
    }
    virtual IConnEventHandler* getConnEventHandler()
    {
    	return _eventHandler;
    }
};

#endif
