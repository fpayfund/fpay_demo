#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "Header.h"
#include "ByteStream.h"

enum PROTO_T {
    PROTO_JSON=0,
    PROTO_BIN,
    PROTO_HTTP,
    PROTO_XML,
    PROTO_NONE,
    PROTO_MAX,
};

struct Request : public Header
{
public:
    Request();
    virtual ~Request();
    Request(const char *data, uint32_t sz, PROTO_T proto = PROTO_BIN);
    Request(const char *data, uint32_t sz, bool copy, PROTO_T proto = PROTO_BIN);
    virtual void popHeader();
    virtual uid_t getUid()
    {
        return 0;
    }
    virtual rkey_t getKey()
    {
        return 0;
    }
    void * getCmd()
    {
        return cmd;
    }
    virtual void setFormHandler(IFormHandle *h);
    proto::BinReadStream& getPackData()
    {
        return up;
    };
    void leftPack(std::string &out);
    static uint32_t peekUint32(const void * d) 
    {
        return XHTONL(*((uint32_t*)d));
    }
    static uint64_t peekUint64(const void * d) 
    {
        return XHTONLL(*((uint64_t*)d));
    }
    static PROTO_T getProtoType(uint32_t len, uint32_t& realLength);

    proto::BinReadStream up;
    void * cmd;
    IFormHandle *parser;
    char* cpBuffer;
    const char *od; //origin data
    uint32_t os;    //origin size
    PROTO_T protoType;
};

#endif
