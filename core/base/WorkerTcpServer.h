#ifndef __WORKER_TCP_SERVER_H_
#define __WORKER_TCP_SERVER_H_

#include <set>
#include <vector>

#include "TcpAcceptor.h"
#include "IAbstractServer.h"

class WorkerTcpServer;

class WorkerTcpAcceptor: public TcpAcceptor {
protected:
    WorkerTcpServer *_server;
public:
    WorkerTcpAcceptor(WorkerTcpServer *tcpServer, int fd)
        : TcpAcceptor(fd, SOCKOPT_NONBLOCK)
        , _server(tcpServer)
    {}
    virtual ~WorkerTcpAcceptor()
    {}
    virtual void doEvent(int ev);
    virtual bool onRecvFd(int channel, int &fd);
    virtual void onAccept(int client, uint32_t ip, int port);
    void start();
};  

class WorkerTcpServer: public IAbstractServer
{
public:
    WorkerTcpServer(int fd)
         : _acceptor(NULL)
         , _sockPairFd(fd)
    {}
    virtual ~WorkerTcpServer()
    {
        if (_acceptor) {
            delete _acceptor;
        }
    }

    void onAccept(int so, uint32_t ip, int port);

    virtual std::string getIp()
    {
        return "";
    }

    virtual uint16_t getPort()
    {
        return 0;
    }

    virtual void start();

private:
    WorkerTcpAcceptor *createAcceptor(int fd);

    WorkerTcpAcceptor* _acceptor;
    int _sockPairFd;
};

#endif
