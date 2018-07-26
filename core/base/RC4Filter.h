#ifndef __RC4_FILTER_H__
#define __RC4_FILTER_H__

#ifdef TLS_ON
#include <openssl/rc4.h>
#include <openssl/sha.h>
#endif

struct RC4Alloc
{
    RC4Alloc();
    ~RC4Alloc();

    unsigned char *writeBuf;
    size_t sz;

    unsigned char *getWriteBuffer()
    {
        return writeBuf;
    }
    size_t getCurSize()
    {
        return sz;
    }
    void realloc(size_t sz);
};  

class RC4Filter
{
public:
    bool encrypted;
#ifdef TLS_ON
    RC4_KEY rc4;
#endif
    static RC4Alloc writeBuffer;

    RC4Filter();
    ~RC4Filter();
    void filterRead(char *, size_t);
    void filterRead(char *, char*, size_t);
    char* filterWrite(char *, size_t);
    bool isEncrypto() const;
    void setRC4Key(const unsigned char *, size_t);
};

#endif
