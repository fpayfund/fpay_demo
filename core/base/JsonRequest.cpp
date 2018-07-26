#include <iostream>
#include "JsonRequest.h"

using namespace std;

JsonRequest::JsonRequest(const char *data, uint32_t sz, PROTO_T proto)
    : Request(data, sz, proto)
{
}

JsonRequest::~JsonRequest()
{
}

void JsonRequest::popHeader()
{
    switch (protoType) {
    case PROTO_JSON:
    case PROTO_BIN: {
            sessionId = up.readUint32();
            len = up.readUint32();
            uri = up.readUint8();
        }
        break;
    default:
        break;
    }
}

PROTO_T JsonRequest::getProtoType(uint64_t hdr, uint32_t& realLength)
{
    sessionId_t session = hdr >> 32;
    return Request::getProtoType((uint32_t)hdr, realLength);
}
