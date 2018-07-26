#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "logging.h"
#include "MasterConn.h"
#include "TcpServerMaster.h"

using namespace std;

void MasterAcceptor::onAccept(int so, uint32_t ip, int port)
{
    DLOG_TRACE;
    master->onAccept(so, ip, port);
}

void MasterAcceptor::start()
{
    DLOG_TRACE;
    addEvent(0, EVENT_READ);
}


TcpServerMaster::TcpServerMaster()
    : _enableIpFilter(false)
{
}

TcpServerMaster::TcpServerMaster(unsigned short port, uint8_t numWorker, const char* exec, const char * workerConf)
    : IWorkerManager(numWorker, exec, workerConf)
    , _listenPort(port)
    , _enableIpFilter(false)
{
}

void TcpServerMaster::onAccept(int so, uint32_t ip, int port)
{
    DLOG_TRACE;
    if (_enableIpFilter) {
        if(!_ipWhiteList.empty() && _ipWhiteList.find(ip) == _ipWhiteList.end()) {
            LOG_WARN << "deny connection from :" << addr_ntoa(ip);
            close(so);
            return;
        }           
    }           
    LOG_INFO << "Accept from from :" << addr_ntoa(ip) << ":" << port;
    MasterConn *conn = (MasterConn*)_connManager->createServerSideConn(so, ip, port, getProtoConsumer(), getConnEventHandler());
    conn->setWorkerManager(this);
}

TcpServerMaster::~TcpServerMaster()
{
    delete _acceptor;
    _acceptor = NULL;
}

MasterAcceptor *TcpServerMaster::createAcceptor(const char *ip, uint16_t port)
{
    DLOG_TRACE;
    try {
        Socket sock; 
        sock.socket();
        sock.setreuse();
        sock.setblocking(false);
        sock.setnodelay();
        sock.bind(ip, port);
        MasterAcceptor *acceptor = new MasterAcceptor(this, sock.detach());
        LOG_INFO << "create listen port " << _listenPort << " success";
        return acceptor;
    } catch(socketExn se) {
        LOG_WARN << "alloc port conflict port:" << _listenPort;
    }

    return NULL;
}

void TcpServerMaster::setIPWhiteList( const std::set<uint32_t> &ipSet )
{
    _ipWhiteList = ipSet;
}

void TcpServerMaster::start()
{
    if (_listenPort != 0) {
        _acceptor = createAcceptor(NULL, _listenPort);
        if (!_acceptor) {
            LOG_WARN << "acceptor not available";
            return;
        }

        _acceptor->start();
    }

    startWorker();
}
