#include <iostream>
#include <arpa/inet.h>
#include "ForwardBuf.h"
#include "JsonProtoConsumer.h"
#include "JsonRequest.h"
#include "logging.h"

using namespace std;
using namespace proto;

int JsonProtoConsumer::onData(const char* buf, size_t size, IConn *conn, int type)
{
    ForwardBuf fb(buf, size);
    while (!fb.empty()) {   
        if (fb.size() < JSON_HEADER_SIZE) {
            LOG_INFO << " need more data, size:" << fb.size();
            break; // need more
        }

        char* dataHdr = (char*)fb.data();
        uint32_t session = Request::peekUint32(fb.data());
        uint32_t *dataSize = (uint32_t*)(4 + (char *)fb.data());
        uint32_t len = XNTOHL(*dataSize);
        uint32_t realLen = 0;
        PROTO_T proto = Request::getProtoType(len, realLen);
        realLen += JSON_HEADER_SIZE;

            LOG_INFO << " need more data, session=" << session << " proto=" << proto 
                << " proto len:" << len
                << " ntohl len:" << ntohl(*dataSize)
                << " xntohl len:" << XNTOHL(*dataSize)
                << " raw len:" << (*dataSize)
                << " realLen:" << realLen
                << " fb size:" << fb.size();

        if (fb.size() < realLen) {
            LOG_INFO << " need more data, session=" << session << " proto=" << proto 
                << " proto len:" << len
                << " ntohl len:" << ntohl(*dataSize)
                << " xntohl len:" << XNTOHL(*dataSize)
                << " raw len:" << (*dataSize)
                << " fb size:" << fb.size();
            break;
        }

        JsonRequest request(dataHdr, realLen, proto);
        request.popHeader();
        if (-1 == doRequest(request, conn)) {
            return -1;
        }

        fb.erase(realLen);
    }

    return (int)fb.offset();
}
