#include <iostream>
#include "MsgProtoConsumer.h"
#include "ForwardBuf.h"
#include "Request.h"
#include "logging.h"

using namespace std;

int MsgProtoConsumer::onData(const char* buf, size_t size, IConn *conn, int type)
{
    ForwardBuf fb(buf, size);
    while (!fb.empty()) 
    {   
        if (fb.size() < HEADER_SIZE)
        {
            LOG_INFO << " need more data, size:" << fb.size();
            break; // need more
        }
        char* dataHdr = (char*)fb.data();
        uint32_t len= Request::peekUint32(fb.data());
        uint32_t realLen = 0;
        PROTO_T proto = Request::getProtoType(len, realLen);
        if (fb.size() < realLen)
        {
            LOG_INFO << " need more data, proto len:" << realLen << " fb size:" << fb.size();
            break;
        }
        Request request(dataHdr, realLen, proto);
        request.popHeader();
        if(-1 == doRequest(request, conn))
        {
            return -1;
        }
        fb.erase(realLen);
    }
    return (int)fb.offset();
}

int MsgProtoConsumer::doRequest(Request &req, IConn *conn)
{
    IWriter *writer = _appContext->requestDispatch(req, conn);
    if(writer) {
        return writer->flush(conn);
    } else {
        return 0;
    }
    return 0;
}
