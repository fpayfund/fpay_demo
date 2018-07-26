#ifndef __HEADER_H__
#define __HEADER_H__

#include "typedef.h"
#include "ServiceContext.h"

#define HEADER_SIZE  9 //(session 4 + len 4  + uri 1)
#define JSON_HEADER_SIZE 8 //(session 4 + len 4(uri 1 + body size))

struct Header
{
public:
    virtual ~Header() {}  
    uint32_t getLength()
    {
        return len;
    }
    virtual void popHeader()
    {}
    virtual void setFormHandler(IFormHandle *h)
    {}
    void setResCode(resCode_t res)
    {
        resCode = res;
    }
    virtual size_t headerSize()
    {   
        return HEADER_SIZE; 
    }   
    virtual uri_t getUri() { return uri;}
    virtual void setUri(uri_t u) { uri = u;} 

protected:
    size_t      len;
    uri_t       uri;
    resCode_t   resCode;
    appId_t     appId;
};

#endif
