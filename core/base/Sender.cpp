#include <iostream>
#include "Sender.h"

Sender::Sender():pb(), hpk(pb), pk(pb, HEADER_SIZE) 
{
}

Sender::Sender(uint32_t head_size) : pb(), hpk(pb), pk(pb, head_size)
{
}

Sender::Sender(const Sender &s): pb(),  hpk(pb), pk(pb, HEADER_SIZE)
{
    len = s.len;
    uri = s.uri;
    resCode = s.resCode;
    pk.writeByte(s.pk.data(), s.pk.size());
}

Sender::Sender(uri_t u, const proto::Marshallable &m) :
    pb(), hpk(pb), pk(pb, HEADER_SIZE)
{
    marshall(u, m);
}

Sender::Sender(uri_t u, const std::string &s) :
    pb(), hpk(pb), pk(pb, HEADER_SIZE)
{
    marshall(u, s);
}

void Sender::endPack()
{
    hpk.replaceUint32(0, len);

    hpk.replaceUint32(4, uri);
    hpk.replaceUint16(8, appId);

    hpk.replaceUint32(0, (uint32_t)(headerSize() + bodySize()));
}

const char *Sender::body()
{
    return pk.data();
}

size_t Sender::bodySize() 
{
    return pk.size();
}

const char *Sender::header()
{
    return hpk.data();
}

size_t Sender::headerSize()
{
    return HEADER_SIZE;
}

void Sender::clear() 
{
    // be care paket offset
    pb.resize(0 + HEADER_SIZE);
    resCode = RES_OK;
    appId = 0;
}

void Sender::marshall(const proto::Marshallable &m)
{
    pk << m;
}

void Sender::marshall(uri_t u, const proto::Marshallable &m)
{
    setUri(u);
    m.marshal(pk);
}

void Sender::marshall(uri_t u, const std::string &s)
{
    setUri(u);
    pk.writeByte(s.data(), s.size());
}

void Sender::marshall(const char *data, size_t sz)
{
    pk.writeByte(data, sz);
}

Sender & Sender::operator= (const Sender& sender)
{
    len = sender.len;
    uri = sender.uri;
    resCode = sender.resCode;
    pb.resize(0);
    pk.writeByte(sender.pk.data(), sender.pk.size());
    return *this;
}
