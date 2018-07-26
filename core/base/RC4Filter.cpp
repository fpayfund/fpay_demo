#include <iostream>
#include "RC4Filter.h"

using namespace std;

#define ENC_BUF_LEN 1024

RC4Alloc::RC4Alloc()
{
    writeBuf = new unsigned char[ENC_BUF_LEN];
    sz = ENC_BUF_LEN;
}

RC4Alloc::~RC4Alloc()
{
    delete[] writeBuf;
}

void  RC4Alloc::realloc(size_t size)
{
    delete[] writeBuf;
    writeBuf= new unsigned char[size];
    sz = size;
}

RC4Alloc RC4Filter::writeBuffer;

RC4Filter::RC4Filter()
{
    encrypted = false;
}

RC4Filter::~RC4Filter()
{

}

void RC4Filter::filterRead(char *data, size_t sz)
{
    if (encrypted) {
#ifdef TLS_ON
        RC4(&rc4, (unsigned long)sz, (unsigned char *)data, (unsigned char *)data);
#endif
    }
}

void RC4Filter::filterRead(char *indata, char* outdadta, size_t sz) 
{
    if (encrypted) {
#ifdef TLS_ON
        RC4(&rc4, (unsigned long)sz, (unsigned char *)indata, (unsigned char *)outdadta);
#endif
    }
}

char* RC4Filter::filterWrite(char *data, size_t sz)
{
    if (encrypted) {
        if (sz > writeBuffer.getCurSize()) {
            writeBuffer.realloc(sz);
        }
#ifdef TLS_ON
        RC4(&rc4, (unsigned long)sz, (unsigned char *)data, writeBuffer.getWriteBuffer());
#endif
        return (char *)writeBuffer.getWriteBuffer();
    } else {
        return data;
    }
}

bool RC4Filter::isEncrypto() const
{
    return encrypted;
}

void RC4Filter::setRC4Key(const unsigned char *data, size_t len)
{
#ifdef TLS_ON
    RC4_set_key(&rc4, (int)len, data);
#endif
    encrypted = true;
}
