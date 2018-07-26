#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__

#include "Socket.h"
#include "IOLoop.h"
#include "env.h"

class EventHandler
{
public:
    virtual void doEvent(int event) {}
    virtual void destroy() { delete this; }
    EventHandler() {}

    enum { INFTIMO = -1, MAXTIMO = (size_t(-1) >> 1) };
    void addTimer(int timeout = INFTIMO);

    virtual std::ostream & trace(std::ostream & os) const;
    std::string toString() const;

protected:
    virtual ~EventHandler() {}
    EventHandler(const EventHandler &) {}
    void operator = (const EventHandler &);
};

inline std::ostream & operator << (std::ostream & os, const EventHandler & h)
{
    return h.trace(os);
}

class SocketHandler : public EventHandler
{
public:
    //virtual void close();
    virtual void addEvent(int remove, int add);
    virtual void removeEvent();
    virtual std::ostream& trace(std::ostream & os) const;

    Socket& socket() { return m_socket; }

protected:
    virtual ~SocketHandler(); 
    Socket m_socket;
};

#endif
