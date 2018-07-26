#include <stdio.h>
#include <iostream>
#include "logging.h"

#include "Connection.h"

using namespace std;

void Connection::sendBin(const char * data, size_t size, uri_t uri)
{
    if (!bEnable) {
        LOG_INFO << "socket not enable:" << addr_ntoa(peerIp);
        return;
    }

    char *dt = const_cast<char *>(data);

    if (socket().isConnected()) {
        _output.write(socket(), dt, size);
        if (!_output.empty()){
            addEvent(0, EVENT_WRITE);
        }
    } else {
        _output.write(dt, size);
        LOG_INFO << "socket not conntected !";
    }
}

void Connection::onConnected(const std::string& errstr)
{
    if (!socket().isConnected()) {
        LOG_WARN << "Connect " << errstr.data() << " to socket" << toString().data();
        _eventHandler->onError(-1, "", this);
        return;
    }

    LOG_INFO << "connected";
    //del conn timer for timeout 
    addTimer();
    int add = EVENT_READ;
    if (!_output.empty()) {
        _output.flush(socket());
        if (!_output.empty())
            add |= EVENT_WRITE;
    }
    addEvent(EVENT_ALL, add);
    _eventHandler->onConnected(this);
}

void Connection::onRead()
{
    try {
        if (_input.pump(socket()) > 0) {
            int ret = _protoHandler->onData(_input.data(), _input.size(), this, _input.isEncrypto()? TCP_ENCODE : TCP_UNCODE);
            if (ret != -1) {
                _input.erase(0, ret);
            } else {
                _eventHandler->onInitiativeClose(this);
            }
        } else {
            _eventHandler->onClose(this);
        }
    } catch(socketExn &se) {
        if (se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
            return;
        }
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + se.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in socketExn:" << err.what();
        }
    } catch(std::exception &ex) {
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + ex.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in exception:" << err.what();
        }
    }
}

void Connection::onWrite()
{
    try {
        _output.flush(socket());
        if (_output.empty())
            addEvent(EVENT_WRITE, 0);
    } catch(socketExn &se) {
        if(se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
            return;
        } else {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn write error:") + se.what()).data(), this);
        }
    } catch(std::exception &ex) {
        _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn write error:") + ex.what()).data(), this);
    }
}

std::ostream & Connection::trace(std::ostream & os) const
{
    return os << " " << addr_ntoa(peerIp) << ":" << peerPort;
}

Connection::~Connection()
{
    LOG_INFO << "Destroy: " << toString().data();
}

void Connection::setTimeout(int timeout)
{
    AbstractConn::setTimeout(timeout);
    addTimer(timeout);
}

void Connection::onTimeout()
{
    if (socket().clearRecvTag()) {
        addTimer(_timeout);
        return;
    }
    try {
        _eventHandler->onTimeout(this);
    } catch(socketExn &se) {
        if(se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
            return;
        }
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + se.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in socketExn:" << err.what();
        }
    } catch(std::exception &ex) {
        try {
            _eventHandler->onError(env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + ex.what()).data(), this);
        } catch(std::exception &err) {
            LOG_WARN << "ignore exception in exception:" << err.what();
        }
    }
}

void Connection::setEncKey(const unsigned char *key, size_t len)
{
    _output.setRC4Key(key, len);
    _input.setRC4Key(key, len);
}

bool Connection::isInputEncrypto() const
{
    return _input.isEncrypto();
}

bool Connection::isOutputEncrypto() const
{
    return _output.isEncrypto();
}

size_t Connection::get_input_buffer_size()
{
    return _input.size();
}

size_t Connection::get_output_buffer_size()
{
    return _output.size();
}
