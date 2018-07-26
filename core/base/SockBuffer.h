#ifndef __SOCK_BUF_H__
#define __SOCK_BUF_H__

#include <stdexcept>
#include "Socket.h"
#include "BlockBuffer.h"

namespace proto
{

class buffer_overflow : public std::runtime_error
{
public:
    buffer_overflow(const std::string& _w)
        : std::runtime_error(_w)
    {}
};

class buffer_overflow_all : public buffer_overflow
{
public:
    buffer_overflow_all(const std::string& _w)
        : buffer_overflow(_w)
    {}
};

class FilterDefault
{
protected:
    void filterRead(char *, size_t)  {}
    char *filterWrite(char *data, size_t) {return data;}
};

template < class BlockBufferClass, class FilterClass = FilterDefault >
struct SockBuffer
    : public BlockBufferClass
    , public FilterClass
{
    using BlockBufferClass::npos;
    using BlockBufferClass::freespace;
    using BlockBufferClass::blocksize;
    using BlockBufferClass::block;
    using BlockBufferClass::max_blocks;
    using BlockBufferClass::tail;
    using BlockBufferClass::size;
    using BlockBufferClass::increase_capacity;
    using BlockBufferClass::append;
    using BlockBufferClass::empty;
    using BlockBufferClass::data;
    using BlockBufferClass::erase;

    using FilterClass::filterWrite;

    typedef FilterClass Filter;

    int pump(Socket & so, size_t n = npos)
    {
        if (freespace() < (blocksize() >> 1) 
                && block() < max_blocks) {
            // ignore increase_capacity result.
            increase_capacity(blocksize());
        }

        size_t nrecv = freespace();
        if (nrecv == 0) return -1;
        if (n < nrecv) nrecv = n; // min(n, freespace());

        int ret = so.recv(tail(), nrecv);
        if (ret > 0) {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
            this->filterRead(tail(), ret);
#else 
            filterRead(tail(), ret);
#endif
            size(size() + ret);
        }
        return ret;
    }

    ////////////////////////////////////////////////////////////////////
    // append into buffer only
    void write(char * msg, size_t size)
    {
        if (size == 0) return;

        char *enc = filterWrite(msg, size);
        if (!append(enc, size))
            throw buffer_overflow_all("output buffer overflow [all]");
    }

    void write(Socket & so, SockBuffer & buf)
    {
        write(so, buf.data(), buf.size());
        buf.erase();
    }

    void write(Socket & so, char * msg, size_t size)
    {	// write all / buffered
        if (size == 0)
            return;

        char *enc = filterWrite(msg, size);

        size_t nsent = 0;
        if (empty()) nsent = so.send(enc, (size_t)size);
        if (!append(enc + nsent, size - nsent)) {
            // all or part append error .
            if (nsent > 0) 
                throw buffer_overflow("output buffer overflow");
            else 
                throw buffer_overflow_all("output buffer overflow [all]");
        }
    }

    int flush(Socket & so, size_t n = npos)
    {
        size_t nflush = size(); if (n < nflush) nflush = n; // std::min(n, size());
        int ret = so.send(data(), nflush);
        erase( 0, ret ); // if call flush in loop, maybe memmove here
        return ret;
    }

}; // SockBuffer

} // namespace

#endif
