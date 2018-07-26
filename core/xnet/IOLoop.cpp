#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "logging.h"
#include "TimerList.h"
#include "EventHandler.h"
#include "IOLoop.h"

using namespace std;

#define TIME_CLICK 500

static int pipe_err = 0;

static void epoll_pipe_handler(int)
{
    pipe_err++;
}

void epoll_timer_click(int)
{
    if (env::ioLoop()) {
        env::ioLoop()->increase_elapsed(TIME_CLICK);
    }
}

EventEpoll::EventEpoll(int size)
{
    struct itimerval timer;

    timer.it_interval.tv_sec = TIME_CLICK / 1000;
    timer.it_interval.tv_usec = (TIME_CLICK % 1000) * 1000;
    timer.it_value = timer.it_interval;

    if (signal(SIGPIPE, epoll_pipe_handler) == SIG_ERR) {
        throw Exception("signal");
    }   

    if (signal(SIGALRM, epoll_timer_click) == SIG_ERR) {
        throw Exception("signal");
    }

    if (-1 == setitimer(ITIMER_REAL, &timer, NULL)) {
        throw Exception("setitimer");
    }

    _epollFd = epoll_create(size);

    if (-1 == _epollFd) {
        throw Exception("epoll");
    } 
}

EventEpoll::~EventEpoll()
{
    if (-1 != _epollFd) {
        close(_epollFd);
    }
}

int EventEpoll::poll(int timeout, epoll_event *events, int sz) 
{
    int waits = epoll_wait(_epollFd, events, sz, timeout);
    if (waits < 0) {
        if (EINTR == errno) {
            return waits;
        }
        DLOG_TRACE << "epoll error wait:" << waits << " errno:" << errno << ", " << strerror(errno);
    }

    for (int i = 0; i < waits; ++i) {
        SocketHandler *sk = (SocketHandler *)events[i].data.ptr;
        if (events[i].events & (EPOLLIN|EPOLLERR|EPOLLHUP)) {
            sk->doEvent((int)EVENT_READ);
        }
        if (events[i].events & EPOLLOUT) {
            sk->doEvent((int)EVENT_WRITE);
        }
    }

    return waits;
}

static void setupEpoll(int efd, int op, int sfd, epoll_event &ev)
{
    int ret = epoll_ctl(efd, op, sfd, &ev);
    if (ret == 0) {
        return;
    }

    switch (errno) {
        case EBADF:
            LOG_INFO << "epfd or fd is not a valid file descriptor.";
            return;
        case EEXIST:
            LOG_INFO << "was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in epfd.";
            return;
        case EINVAL:
            LOG_INFO << "epfd is not an epoll file descriptor, or fd is the same as epfd, or the requested operation op is not supported by this interface.";
            return;
        case ENOENT:
            LOG_INFO << "op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in epfd. ";
            return;
        case ENOMEM:
            LOG_INFO << "There was insufficient memory to handle the requested op control operation.";
            return;
        case EPERM:
            LOG_INFO << "The target file fd does not support epoll.";
            return;
    }
}

int EventEpoll::addEventHandler(int remove, int add, SocketHandler *s)
{
    DLOG_TRACE;
    epoll_event ev;
    ev.data.ptr = (void*)s;
    ev.events = EPOLLIN;

    std::pair<sockSet_t::iterator, bool> p = _sockets.insert((SocketHandler*)s);
    if (p.second) {
        if (EVENT_WRITE & add) {
            ev.events |= EPOLLOUT;
        }
        setupEpoll(_epollFd, EPOLL_CTL_ADD, s->socket().getsocket(), ev);
    } else {
        unsigned int currstate = (s->socket().m_sock_flags.selevent & ~remove) | add;
        ev.events=0;
        if (currstate & EVENT_READ)  ev.events |= EPOLLIN;
        if (currstate & EVENT_WRITE) ev.events |= EPOLLOUT;
        setupEpoll(_epollFd, EPOLL_CTL_MOD, s->socket().getsocket(), ev);
    }
    return 0;
}

int EventEpoll::removeEventHandler(SocketHandler *s)
{
    DLOG_TRACE;
    epoll_event ev;
    setupEpoll(_epollFd, EPOLL_CTL_DEL, s->socket().getsocket(), ev);
    _sockets.erase(s);
    return 0;
}

IOLoop::IOLoop():_eventEngine(NULL), _timerMgmt(NULL), m_elapsed(0) 
{
    start();
    _status = kInitialized;
}

IOLoop::~IOLoop() 
{
    if (_eventEngine) {
        delete _eventEngine;
        _eventEngine = NULL;
    }
    if (_timerMgmt) {
        delete _timerMgmt; 
        _timerMgmt = NULL;
    }
}

void IOLoop::start()
{
    _eventEngine = new EventEpoll(65535);
    _timerMgmt = new TimerList();
}

void IOLoop::run()
{
    DLOG_TRACE;
    time_t oldtime = env::now;
    epoll_event events[EPOLL_SIZE_MAX];

    _status = kRunning;

    while (isRunning()) {
        int wait = _eventEngine->poll(TIME_CLICK - 1, events, EPOLL_SIZE_MAX);

        env::now = time(NULL);

        struct timeval stv;
        struct timezone stz;
        gettimeofday(&stv,&stz);
        env::now = stv.tv_sec;
        env::msec = stv.tv_sec * 1000 + stv.tv_usec / 1000;
        env::usec = ((uint64_t)stv.tv_sec) * 1000 * 1000 + stv.tv_usec;

        if (oldtime != env::now) {
            char buf[30];
            char *st = ctime_r(&env::now, buf);
            env::strTime = std::string(st, strlen(st) - 1);
        } 

        checkTimeout(); 
    }
}

int IOLoop::addTimer(int timeout, EventHandler *h) 
{
    return _timerMgmt->addTimer(timeout, h);
}

void IOLoop::doEvent(EventHandler *s, int event)
{
    return s->doEvent(event);
}

int IOLoop::checkTimeout()
{
    //get timeout call
    std::vector<EventHandler*> v;
    int n = _timerMgmt->checkTimer(m_elapsed, v);
    resetElapsed();

    std::vector<EventHandler*>::iterator it = v.begin(); 
    for (; it != v.end(); it ++) {
        EventHandler *s = *it;
        doEvent(s, (int)EVENT_TIMEOUT);
    }

    return n;
}

void IOLoop::stop()
{
    assert(_status == kRunning);
    _status = kStopping;
    _substatus = kStoppingListener;
}

