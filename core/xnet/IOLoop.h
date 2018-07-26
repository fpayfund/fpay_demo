#ifndef __IO_LOOP_H__
#define __IO_LOOP_H__

#include <set>
#include <vector>
#include <sys/epoll.h>
#include "ServerStatus.h"

#define EPOLL_SIZE_MAX 1024

class SocketHandler;
class EventHandler;

class Reactor
{
public:
    Reactor() {}
    virtual ~Reactor() {}
    virtual int poll(int timeout, epoll_event* events, int sz=EPOLL_SIZE_MAX) = 0;
    virtual int addEventHandler(int remove, int add, SocketHandler *h) = 0;
    virtual int removeEventHandler(SocketHandler *s) = 0;
};

class EventEpoll: public Reactor
{
public:
    EventEpoll(int size);
    ~EventEpoll(); 
    virtual int poll(int timeout, epoll_event* events, int sz=EPOLL_SIZE_MAX);
    virtual int addEventHandler(int remove, int add, SocketHandler *h);
    virtual int removeEventHandler(SocketHandler *s);

private:
    typedef std::set<SocketHandler*> sockSet_t;
    sockSet_t _sockets;
    int _epollFd;
};

class TimerMgmt;

class IOLoop : public ServerStatus
{
public:
    IOLoop();
    virtual ~IOLoop();

    virtual void start();
    virtual void run();
    virtual void stop();

    virtual int addEventHandler(int remove, int add, SocketHandler *h) 
    {
        return _eventEngine->addEventHandler(remove, add, h);
    }
    virtual int removeEventHandler(SocketHandler *s)
    {
        return _eventEngine->removeEventHandler(s);
    }
    void doEvent(EventHandler *s, int event);

    virtual int addTimer(int timeout, EventHandler *h);
    void timout_run(int elapsed);
    void increase_elapsed(int elapsed)
    {
        m_elapsed += elapsed;
    }
    void resetElapsed()
    {
        m_elapsed = 0;
    }

    int checkTimeout();

private:
    Reactor* _eventEngine;
    TimerMgmt* _timerMgmt; 
    int m_elapsed;
};
#endif

