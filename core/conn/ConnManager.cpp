#include <iostream>
#include <sstream>
#include <stdio.h>

#include "logging.h"
#include "ConnManager.h"

using namespace std;

#define CONN_DEL_DELAY_TIME 10

IConn* LazyDelConnManager::createServerSideConn(int socket, uint32_t ip, int port,
        IProtoConsumer *consumer, IConnEventHandler *eventHandler)
{
    DLOG_TRACE;
    IConn *conn = _serverConnFactory->createConnection(socket, ip, port, consumer, eventHandler, this);
    return conn;
}   

IConn* LazyDelConnManager::createClientSideConn(const std::string& ip, uint32_t port,
        IProtoConsumer *consumer, IConnEventHandler *eventHandler)
{
    DLOG_TRACE;
    IConn *conn = _clientConnFactory->createConnection(ip, port, consumer, eventHandler, this);
    return conn;
} 

void ConnManager::onConnected(IConn *conn) 
{
    DLOG_TRACE;
}

void ConnManager::onClose(IConn *conn)
{
    DLOG_TRACE << "[ConnManagerLinkImp::onClose] conn id:" << conn->getConnId();
    eraseConnection(conn);
}

void ConnManager::onInitiativeClose(IConn *conn){
    if(conn) {
        DLOG_TRACE << "[ConnManagerLinkImp::onInitiativeClose] conn id:" << conn->getConnId();
    }
    eraseConnection(conn);
}

void ConnManager::onError(int ev, const char* msg, IConn* conn)
{
    std::string strPeer;
    if (conn) {
        std::stringstream ss;
        ss << addr_ntoa(conn->getPeerIp()) << ":" << conn->getPeerPort();
        strPeer = ss.str();
    }

    DLOG_TRACE << "onError]: Event = " << ev 
        << ", msg=" << msg 
        << ", cid=" << (conn ? conn->getConnId(): -1)
        << ", peer" << strPeer;

    eraseConnection(conn);
}

void ConnManager::onTimeout(IConn *conn)
{
    DLOG_TRACE << "onTimeout]: connid=" << (conn ? conn->getConnId() : -1);
    eraseConnection(conn);
}

LazyDelConnManager::LazyDelConnManager()
{
    addTimer(CONN_DEL_DELAY_TIME);
}

void LazyDelConnManager::doEvent(int event)
{
    if (!lazyDelIds.empty()) {
        for (std::set<uint32_t>::iterator it = lazyDelIds.begin(); it != lazyDelIds.end(); ++it) {
            LOG_INFO << "doEvent delete conn=" << *it;
            IConn *conn = getConnById(*it);
            if (conn && !conn->isEnable()) {
                eraseConnection(conn);
            }
        }

        lazyDelIds.clear();
    }

    addTimer(CONN_DEL_DELAY_TIME);
}

void LazyDelConnManager::DelayDelConn(IConn* conn)
{
    if (conn) {
        conn->setEnable(false);
        lazyDelIds.insert(conn->getConnId());
    }
}
