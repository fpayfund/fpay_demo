#include <iostream>
#include <sstream>
#include "EventHandler.h"
using namespace std;

void EventHandler::addTimer(int timeout)
{
    IOLoop* loop = env::ioLoop();
    if (loop) {
        loop->addTimer(timeout, this);
    }
}

std::ostream & EventHandler::trace(std::ostream & os) const
{
    return os;
}

std::ostream & SocketHandler::trace(std::ostream & os) const
{
    try {   
        std::string ip; 
        int port = 0;
        if (m_socket.isConnected())
            ip = m_socket.getpeerip(&port);

        return os << '-' << (unsigned int)m_socket.getsocket() << ' ' << ip << ':' << port;
    } catch ( std::exception & ex) {   
        return os << ex.what();
    }   
}

void SocketHandler::addEvent(int remove, int add)
{
    if (!socket().isValid()) {
        throw std::runtime_error("select with invalid socket");
    }

    if (socket().isShutdownSends()) {
        add &= ~EVENT_WRITE;
    }

    if (socket().isShutdownReceives()) {
        add &= ~EVENT_READ;
    }

    remove = remove & ~add;

    remove &= socket().m_sock_flags.selevent;
    add &= ~socket().m_sock_flags.selevent;

    if ((remove + add) != EVENT_NONE) {
        env::ioLoop()->addEventHandler(remove, add, this);
        socket().m_sock_flags.selevent = (socket().m_sock_flags.selevent & ~remove) | add;
        socket().m_sock_flags.selected = 1;
    }   
}

void SocketHandler::removeEvent()
{
    if (socket().m_sock_flags.selected == 1) {
        env::ioLoop()->removeEventHandler(this);
        socket().m_sock_flags.selected = 0;
    }
}

SocketHandler::~SocketHandler()
{
    removeEvent();
}

std::string EventHandler::toString() const 
{
    std::ostringstream ostr;
    trace(ostr);
    return ostr.str();
}
