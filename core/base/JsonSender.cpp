#include "JsonSender.h"

JsonSender::JsonSender(uri_t uri, const proto::Marshallable &obj)
    : Sender(uri, obj)
    , sessionId(-1)
{
}

JsonSender::JsonSender(sessionId_t s, uri_t uri, const proto::Marshallable &obj)
    : Sender(uri, obj)
    , sessionId(s)
{
    setUri(uri);
    marshall(uri, obj);
}

void JsonSender::marshall(uri_t uri, const proto::Marshallable &m) 
{
    setUri(uri);
    m.marshal(pk);
}

void JsonSender::endPack()
{
    hpk.replaceUint32(0, sessionId);

    hpk.replaceUint32(4, len);
    hpk.replaceUint8(8, uri);
    hpk.replaceUint32(4, sizeof(uri_t) + bodySize());// len not include header size(session + size)
}
