#ifndef __JSON_PROTO_CONSUMER_H__
#define __JSON_PROTO_CONSUMER_H__

#include "MsgProtoConsumer.h"

class JsonProtoConsumer
    : public MsgProtoConsumer 
{
public:
    JsonProtoConsumer() {}
    virtual ~JsonProtoConsumer() {}
    virtual int onData(const char*, size_t, IConn *conn, int type=0);
};

#endif
