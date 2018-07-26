#ifndef _CORE_TCP_SERVER_H_
#define _CORE_TCP_SERVER_H_
#include <vector>
#include <set>
#include "TcpAcceptor.h"
#include "Worker.h"
#include "IAbstractServer.h"

class TcpServerMaster;

class MasterAcceptor
    : public TcpAcceptor
{
public:
    MasterAcceptor(TcpServerMaster *m, int fd)
        : TcpAcceptor(fd)
        , master(m)
    {}
    virtual ~MasterAcceptor()
    {}
    virtual void onAccept(int client, uint32_t ip, int port);
    void start();

protected:
    TcpServerMaster *master;
};  

class TcpServerMaster
    : public IAbstractServer
    , public IWorkerManager
{
protected:
    friend class MasterAcceptor;

    MasterAcceptor* _acceptor;
    uint16_t _listenPort;
    std::string _ip;
    bool _enableIpFilter;
    std::set<uint32_t> _ipWhiteList;

public:
    TcpServerMaster();
    TcpServerMaster(unsigned short port, uint8_t numWorker, const char* exec, const char * workerConf);
    virtual ~TcpServerMaster();

    void onAccept(int so, uint32_t ip, int port);

    void setIPWhiteList(const std::set<uint32_t> &ipSet);
    void setListenPort(const uint16_t port)
    {
        _listenPort = port;
    }

    virtual std::string getIp()
    {
        return _ip;
    }

    virtual uint16_t getPort()
    {
        return _listenPort;
    }

    virtual void start();

protected:
    virtual MasterAcceptor* createAcceptor(const char* ip, uint16_t port);
};

#endif
