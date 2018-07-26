#ifndef _BASE_PROXY_CONN_H_ 
#define _BASE_PROXY_CONN_H_
#include "TcpSock.h"
#include "Worker.h"
#include "AbstractConn.h"
#include "ProtoConsumer.h"

class MasterConn
    : public AbstractConn
    , public TcpSock
    , public IWorkerManagerAware
{
public: 
    enum ConnType {
        ACCEPT,
        CONNECT 
    };

    ConnType cType;
    MasterConn() {}
    MasterConn(int fd, uint32_t ip, int port,
            IProtoConsumer *proto, IConnEventHandler *eh, bool nodelay = false)
        : TcpSock(fd)
    {
        if (nodelay) {
            socket().setnodelay();
        }

        socket().setCloseExec();

        setProtoConsumer(proto);
        setConnEventHandler(eh);
        cType = ACCEPT;

        init("Accept from", ip, port);
   }

    MasterConn(const std::string & ip, int port, int timeout,
            IProtoConsumer *proto, IConnEventHandler *eh, bool nodelay = false)
        : TcpSock(ip, port, timeout)
    {
        if (nodelay) {
            socket().setnodelay();
        }

        setProtoConsumer(proto);
        setConnEventHandler(eh);
        cType = CONNECT;

        init("Connect to", aton_addr(ip), port);
    }

    MasterConn(uint32_t ip, int port, int timeout,
            IProtoConsumer *proto, IConnEventHandler *eh, bool nodelay = false)
        : TcpSock(ip, port, timeout)
    {
        if (nodelay) {
            socket().setnodelay();
        }

        setProtoConsumer(proto);
        setConnEventHandler(eh);
        cType = CONNECT;

        init("Connect to", ip, port);
    }


    virtual ~MasterConn(); 

    uint32_t peekUint32();
    uint64_t peekUint64();
    virtual void sendBin(const char * data, size_t size, uri_t uri) {}
    virtual void setTimeout(int);
    void setEncKey(const unsigned char *key, size_t len) {}

    virtual void onTimeout();

    cid_t connection_id;

protected:
    void init(const char * info, uint32_t ip, int port)
    {
        peerIp = ip;
        peerPort = port;
    }

    virtual void onRead();
    virtual void onWrite()
    {}
    virtual void onConnected(const std::string & errstr)
    {}
    virtual std::ostream & trace(std::ostream & os) const;
};

#endif
