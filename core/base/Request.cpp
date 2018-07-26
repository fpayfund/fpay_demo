#include <iostream>
#include "Request.h"

using namespace std;

Request::Request()
    : up(NULL, 0)
    , cmd(NULL)
    , parser(NULL)
    , cpBuffer(NULL)
    , od(NULL)
    , os(0)
    , protoType(PROTO_NONE)
{ 
}

Request::Request(const char *data, uint32_t sz, PROTO_T proto)
    : up(data, sz)
    , cmd(NULL)
    , parser(NULL)
    , cpBuffer(NULL)
    , od(data)
    , os(sz)
    , protoType(proto) 
{ 
}

Request::Request(const char *data, uint32_t sz, bool copy, PROTO_T proto)
    : up(data, sz)
    , cmd(NULL)
    , parser(NULL)
    , cpBuffer(NULL)
    , od(data)
    , os(sz)
    , protoType(proto) 
{
    if (copy) {
        cpBuffer = new char[sz];
        memcpy(cpBuffer, data, sz);
        up.reset(cpBuffer, sz);
        od = cpBuffer;
    }
}

#define PROTO_MAGIC_SHIFT 29
#define PROTO_MAGIC_MASK  ~(111 << PROTO_MAGIC_SHIFT)
PROTO_T Request::getProtoType(uint32_t len, uint32_t& realLength)
{
    uint32_t top3Bit = len >> PROTO_MAGIC_SHIFT;
    PROTO_T proto = PROTO_MAX;

    switch (top3Bit) {
        case PROTO_JSON:
            proto = PROTO_JSON; 
            break;
        case PROTO_BIN:
            proto = PROTO_BIN; 
            break;
        case PROTO_HTTP:
            proto = PROTO_HTTP; 
            break;
        default:
            break;
    }
    realLength = len & PROTO_MAGIC_MASK;
    return proto;
}

void Request::popHeader()
{
    switch (protoType)
    {
        case PROTO_BIN:
            {
                len = up.readUint32();
                uri = up.readUint32();
                appId = up.readUint16();
            }
            break;
        case PROTO_HTTP:
            break;
        default:
            break;
    }
}

Request::~Request()
{
    if (parser && cmd) {
        parser->destroyForm(cmd);
    }

    if (cpBuffer != NULL) {
        delete[] cpBuffer;
    }
}

void Request::setFormHandler(IFormHandle *h)
{
    parser = h;
    cmd = parser->handlePacket(up);
}

void Request::leftPack(std::string &out)
{
    out.assign(up.data(), up.size());
}
