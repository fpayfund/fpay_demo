#include <iostream>
#include "TcpSock.h"
#include "logging.h"

using namespace std;

TcpSock::~TcpSock()
{
    DLOG_TRACE;
    //del timer
    addTimer();
}

void TcpSock::addEvent(int remove, int add)
{
    if (add & EVENT_CONNECTING) {
        if (socket().isConnected()) {
            onConnected("connect ok immediately");
            return;
        }   

        add = EVENT_RW;
    }   

    SocketHandler::addEvent(remove, add);
}

void TcpSock::doEvent(int ev)
{
    if (socket().isConnected()) {
        switch (ev) {
        case EVENT_READ :
            onRead();
            return;
        case EVENT_WRITE :
            onWrite();
            return;
        case EVENT_TIMEOUT :
            onTimeout();
            return;
        }
    } else {
        switch (ev) {
        case EVENT_TIMEOUT :
            onConnected("connect timeout");
            return;
        case EVENT_READ :
        case EVENT_WRITE :
             onConnected(socket().complete_nonblocking_connect());
             return;
        }
    }

    assert(false);
}

void TcpSock::onTimeout()
{
    destroy();
}

void TcpSock::onConnected(const std::string& errstr)
{
    assert(false);
}
