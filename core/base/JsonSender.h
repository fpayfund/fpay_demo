#ifndef __JSON_SENDER_H__
#define __JSON_SENDER_H__

#include "typedef.h"
#include "Sender.h"

class JsonSender : public Sender
{
public:
    JsonSender(): sessionId(-1) {}
    JsonSender(sessionId_t s, uri_t uri, const proto::Marshallable &);
    JsonSender(uri_t uri, const proto::Marshallable &);
    void marshall(uri_t u, const proto::Marshallable &m);

    virtual size_t headerSize()
    {
        return HEADER_SIZE;
    } 

    virtual void endPack();

private:
    sessionId_t sessionId;
};

#endif
