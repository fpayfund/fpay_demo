#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include "logging.h"
#include "TcpServer.h"

using namespace std;

void StandaloneTcpAcceptor::onAccept(int fd, uint32_t ip, int port)
{
    _server->onAccept(fd, ip, (uint16_t)port);
}

void StandaloneTcpAcceptor::start()
{
    addEvent(0, EVENT_READ);
}

void TcpServer::onAccept(int fd, uint32_t ip, uint16_t port)
{
    LOG_INFO << "Accept from from :" << addr_ntoa(ip) << ":" << port;
    _connManager->createServerSideConn(fd, ip, port, getProtoConsumer(), getConnEventHandler());
}

StandaloneTcpAcceptor* TcpServer::createAcceptor(const char* ip, uint16_t port)
{
    if (!ip || port == 0) {
        return NULL;
    }

    try {
        Socket socket; 
        socket.socket();
        socket.setreuse();
        socket.setblocking(false);
        socket.setnodelay();
        socket.bind(ip, port);

        return new StandaloneTcpAcceptor(this, socket.detach());
    } catch (socketExn se) {
        LOG_WARN << "start tcp server fail: port:" << port;
    }

    return NULL;
}

void TcpServer::start()
{
    _acceptor = createAcceptor("0.0.0.0", _listenPort);

    if(!_acceptor) {
        LOG_WARN << "No ports available!";
        return;
    }

    _acceptor->start();
}
