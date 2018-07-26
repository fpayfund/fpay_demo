#ifndef __SENDER_H__
#define __SENDER_H__

#include "ByteStream.h"
#include "typedef.h"
#include "Header.h"

struct Sender : public Header
{
protected:
    proto::PackBuffer pb; 
    proto::BinWriteStream hpk;
    proto::BinWriteStream pk; 

public:
    Sender();
    Sender(uint32_t head_size);
    virtual ~Sender() {}
    Sender(uri_t, const proto::Marshallable &); 
    Sender(uri_t, const std::string &); 
    Sender & operator = (const Sender& send);
    Sender(const Sender &); 
    virtual const char *body();
    virtual size_t bodySize();
    virtual const char *header();
    virtual size_t headerSize();
    virtual void marshall(const proto::Marshallable &); 
    virtual void marshall(uri_t, const proto::Marshallable &); 
    virtual void marshall(uri_t, const std::string &); 
    virtual void marshall(const char *, size_t sz);
    virtual void endPack();
    virtual void clear();
};

#endif
