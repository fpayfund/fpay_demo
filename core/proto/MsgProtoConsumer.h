#ifndef _BASE_MSG_PROTO_CONSUMER_H_
#define _BASE_MSG_PROTO_CONSUMER_H_
#include "base.h"
#include "ProtoConsumer.h"
#include "ServiceContext.h"

class MsgProtoConsumer
    : public IProtoConsumer 
    , public ServiceContextAware 
{
    public:
        MsgProtoConsumer() {}
        virtual ~MsgProtoConsumer() {}
        virtual void setPackLimit(int sz) { _limitSize = sz;}

        inline uint32_t getPackLimit() const {
            return _limitSize;
        }
        virtual int onData(const char*, size_t, IConn *conn, int type=0);
        virtual int doRequest(Request &request, IConn *rconn);

    private:
        uint32_t _limitSize;
};
#endif
