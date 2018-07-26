#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "TcpSock.h"
#include "AbstractConn.h"
#include "SockBuffer.h"
#include "RC4Filter.h"
#include "ProtoConsumer.h"

class Connection
    : public AbstractConn
    , public TcpSock
{
public: 
    enum ConnType {
        ACCEPT,
        CONNECT 
    };

    ConnType cType;

    Connection() {}

    Connection(int fd, uint32_t ip, int port,
            IProtoConsumer *consumer, IConnEventHandler *handler, bool nodelay = false)
        : TcpSock(fd)
    {
        if (nodelay) {
            socket().setnodelay();
        }
        setProtoConsumer(consumer);
        setConnEventHandler(handler);
        cType = ACCEPT;
        init("Accept from", ip, port);
   }

    Connection(const std::string & ip, int port, int timeout,
            IProtoConsumer *consumer, IConnEventHandler *handler, bool nodelay = false)
        : TcpSock(ip, port, timeout)
    {
        if (nodelay) {
            socket().setnodelay();
        }
        setProtoConsumer(consumer);
        setConnEventHandler(handler);
        cType = CONNECT;
        init("Connect to", aton_addr(ip), port);
    }

    Connection(uint32_t ip, int port, int timeout,  IProtoConsumer *consumer,
            IConnEventHandler *handler, bool nodelay = false)
        : TcpSock(ip, port, timeout)
    {
        if(nodelay){
            socket().setnodelay();
        }
        setProtoConsumer(consumer);
        setConnEventHandler(handler);
        cType = CONNECT;
        init("Connect to", ip, port);
    }

    virtual ~Connection(); 

    virtual void sendBin(const char * data, size_t size, uri_t uri);

    virtual void setEncKey(const unsigned char *key, size_t len);
    virtual bool isInputEncrypto() const;
    virtual bool isOutputEncrypto() const;
    virtual void setTimeout(int);

    virtual void onTimeout();

public:
    size_t get_input_buffer_size();
    size_t get_output_buffer_size();
    cid_t connection_id;

protected:
    void init(const char * info, uint32_t ip, int port)
    {
        peerIp = ip;
        peerPort = port;
    }

    virtual void onRead();
    virtual void onWrite();
    virtual std::ostream & trace(std::ostream & os) const;
    virtual void onConnected(const std::string & errstr);

    typedef proto::BlockBuffer<proto::def_block_alloc_8k, 128*8> Buffer8x9k;
    typedef proto::BlockBuffer<proto::def_block_alloc_8k, 128*8> Buffer8x128k;

    typedef proto::SockBuffer<Buffer8x9k, RC4Filter> InputBuffer;
    typedef proto::SockBuffer<Buffer8x128k, RC4Filter> OutputBuffer;

    InputBuffer _input;
    OutputBuffer _output;
};
#endif
