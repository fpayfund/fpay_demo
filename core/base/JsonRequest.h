#ifndef __JSON_REQ_H__
#define __JSON_REQ_H__

#include "Request.h"

class JsonRequest : public Request
{
public:
    virtual ~JsonRequest();
    JsonRequest(const char *data, uint32_t sz, PROTO_T proto = PROTO_JSON);
    virtual size_t getHeaderSize()
    {
        return JSON_HEADER_SIZE;
    }
    virtual uid_t getUid()
    {
        return sessionId;
    }
    virtual rkey_t getKey()
    {
        return sessionId;
    }
    void popHeader();
    static PROTO_T getProtoType(uint64_t hdr, uint32_t& realLength);

protected:
    sessionId_t sessionId;
};

#endif
