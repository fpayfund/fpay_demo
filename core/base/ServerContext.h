#ifndef __SERVER_CONTEXT_H__
#define __SERVER_CONTEXT_H__
#include <map>
#include "ServiceContext.h"
#include "Request.h"

class ServerContext
    : public ServiceContext
    , public IWriterAware
{
protected:
    typedef std::map<uri_t, FormEntry *> entry_map_t;
    entry_map_t entries;
public:
    virtual IWriter *requestDispatch(Request &request, IConn *conn);
    virtual void addEntry(FormEntry *entries, void *target, IWriterAware *inf);
    void * fetchPtr(FormEntry *entry, Request &request, IConn* conn);
    virtual void setParent(ServiceContext *p) {}
    void dispatchMessage(FormEntry* entry, Request &request, IConn *conn);
    virtual void clear();
protected:
    virtual void DefaultDispatch(Request &request, IConn *conn);
};

#endif
