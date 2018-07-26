#ifndef __STATIC_DIRECT_WRITER_H__
#define __STATIC_DIRECT_WRITER_H__

#include <string>
#include <map>

#include "base.h"
#include "ServiceContext.h"
#include "MultiConnManager.h"

class DirectWriter
    : public IWriter
    , public IFormTarget
    , public IProtoConsumerAware
    , public MultiConnManager
{
public:
    DirectWriter() {}

    virtual void answer(uri_t uri, const proto::Marshallable &obj) { }
    virtual void stop() { }
    virtual int flush(IConn *conn) { return 0;}

    void answer(ConnPtr_t conn, uri_t uri, const proto::Marshallable &obj);
    virtual void route(sid_t sid, uri_t uri, const proto::Marshallable &obj);
    virtual void sendTo(const std::string& ipport, uri_t uri, const proto::Marshallable &obj);

    virtual bool dispatch(sid_t serverId, uint32_t serverIp, uint16_t port, uri_t uri, const proto::Marshallable &); 
    virtual IConn *connectServer(sid_t serverId, uint32_t ip, uint16_t port);
    virtual void eraseConnection(IConn *conn);
private:
    sid_t genSId(uint32_t ip, uint16_t port);
    void getAddress(sid_t sid, std::string& address);
    void getAddressFromSId(sid_t sid, uint32_t &ip, uint16_t &port);

    std::map<sid_t, cid_t> _sid2cid;
};

#endif
