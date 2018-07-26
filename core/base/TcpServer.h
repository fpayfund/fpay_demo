#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <vector>
#include <set>

#include "TcpAcceptor.h"
#include "IAbstractServer.h"

class TcpServer;

class StandaloneTcpAcceptor: public TcpAcceptor
{
protected:
    TcpServer *_server;

public:
    StandaloneTcpAcceptor(TcpServer *server, int fd)
        : TcpAcceptor(fd)
        , _server(server)
    {}
    virtual ~StandaloneTcpAcceptor()
    {}
    virtual void onAccept(int client, uint32_t ip, int port);
    void start();
};  

class TcpServer: public IAbstractServer
{
public:
    TcpServer(int port)
        : _listenPort(port)
        , _acceptor(NULL)
    {}

    virtual ~TcpServer()
    {
        delete _acceptor;
        _acceptor = NULL;
    }

    void onAccept(int fd, uint32_t ip, uint16_t port);

    virtual std::string getIp()
    {
        return _ip;
    }

    virtual void setPort(uint16_t port)
    {
        _listenPort = port;
    }

    virtual uint16_t getPort()
    {
        return _listenPort;
    }

    virtual void start();

private:
    StandaloneTcpAcceptor* createAcceptor(const char *ip, uint16_t port);

    std::string _ip;
    uint16_t _listenPort;
    StandaloneTcpAcceptor* _acceptor;
};

#endif
