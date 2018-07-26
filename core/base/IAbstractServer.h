#ifndef __IABSTRACT_SERVER_H__
#define __IABSTRACT_SERVER_H__

#include "typedef.h"
#include "IServer.h"

class IAbstractServer
    : public virtual IServer
    , public virtual IConnEventHandlerAware
    , public virtual IConnManagerAware
{
protected:
    volatile sid_t _server_id;

public:
    IAbstractServer()
    {}
    virtual ~IAbstractServer()
    {}
    virtual sid_t getServerId()
    {
        return _server_id;
    }
    virtual void start()
    {}
};

#endif
