#include <iostream>
#include <stdio.h>
#include "MasterConn.h"
#include "Worker.h"
#include "logging.h"
using namespace std;

void MasterConn::onRead()
{
    try {
        //peek data, and dispatch req to worker
        bool ret = _workerManager->dispatch(this);
        if (!ret) {
            LOG_ERROR << "dispatch fails";
            _eventHandler->onInitiativeClose(this);
        } else {
            _eventHandler->onClose(this);
        }
    }  catch(socketExn &se) {
        if (se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
            return;
        }

        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Proxy Conn read error:") + se.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in socketExn:" << err.what();
        }
    } catch(std::exception &ex) {
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Proxy Conn read error:") + ex.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in exception:" << err.what();
        }
    }
}

std::ostream & MasterConn::trace(std::ostream & os) const
{
    return os << " "<< addr_ntoa(peerIp) << ":"<< peerPort;
}

MasterConn::~MasterConn()
{
    LOG_INFO << "Destroy: " << toString().data();
}

void MasterConn::setTimeout(int tm)
{
    AbstractConn::setTimeout(tm);
    addTimer(tm);
}

void MasterConn::onTimeout()
{
    if (socket().clearRecvTag()) {
        addTimer(_timeout);
        return;
    }

    try {
        _eventHandler->onTimeout(this);
    } catch(socketExn &se) {
        if (se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
            return;
        }

        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Proxy Conn read error:") + se.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in socketExn:" << err.what();
        }

    } catch(std::exception &ex) {
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Proxy Conn read error:") + ex.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in exception:" << err.what();
        }
    }
}

uint64_t peekData(int _fd, int len)
{
    char buf[len];
    int nRead = recv(_fd, buf, len, MSG_PEEK);
    if (nRead == 0) {
        LOG_ERROR << "conn fd " << _fd << " was closed";
        return -1;
    }

    if (nRead != len) {
        LOG_ERROR << "fd " << _fd << " errno: " << errno << " , Error: " << strerror(errno);
        return -1;
    }

    uint64_t sessionId = 0;
    for (int i = 0; i < len; ++i) {
        sessionId += (uint8_t)(buf[i]) << (24 - 8 * i);
    }

    LOG_DEBUG << "sessionId=" << sessionId;

    return sessionId;
}

uint32_t MasterConn::peekUint32()
{
    return (uint32_t)peekData(socket().getsocket(), sizeof(int));
}

uint64_t MasterConn::peekUint64()
{
    return (uint64_t)peekData(socket().getsocket(), sizeof(uint64_t));
}
