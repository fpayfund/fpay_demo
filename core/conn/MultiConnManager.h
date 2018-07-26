#ifndef __MULTI_CONN_MANAGER_H__
#define __MULTI_CONN_MANAGER_H__

#include <map>
#include "ConnManager.h"

class MultiConnManager: public LazyDelConnManager
{
public:
    MultiConnManager();
    virtual ~MultiConnManager();

    virtual void eraseConnection(IConn *conn);
    virtual void eraseConnectionById(cid_t id);

    virtual bool dispatchById(cid_t cid, Sender &request);
    virtual bool dispatchByIds(const std::set<cid_t> &ids, Sender &request, uint32_t exp);

    virtual IConn *getConnById(cid_t id);

    virtual void onConnCreate(IConn *conn);

protected:
    cid_t cid;
    typedef std::map<cid_t, IConn *> conn_map_t;
    conn_map_t connMap;
};

#endif
