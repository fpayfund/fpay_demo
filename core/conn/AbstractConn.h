#ifndef __ABSTRACT_CONN_H__
#define __ABSTRACT_CONN_H__

#include "base.h"
#include "Sender.h"

class AbstractConn : public IConn
{
public:
    AbstractConn();
    virtual ~AbstractConn() {}
    virtual void send(Sender &resp);
    virtual void setTimeout(int timeout);
    virtual void *getData();
    virtual void notifyErr(int err, const char *msg);

protected:
    int _timeout;
};

#endif
