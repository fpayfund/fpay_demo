#ifndef __TIMER_LIST_H__
#define __TIMER_LIST_H__

#include <list>
#include "TimerMgmt.h"

class TimerNode
{
public:
    TimerNode (int e, int elap, EventHandler* h)
        : expire(e)
        , elapsed(elap)
        , handler(h)
    {}

    int expire;
    int elapsed;
    EventHandler* handler;
};

class TimerList : public TimerMgmt
{
public:
    TimerList()
        : _elapsed(0)
    {}
    virtual ~TimerList()
    {}
    virtual int addTimer(int timeout, EventHandler* handler);
    virtual int checkTimer(int tick, std::vector<EventHandler*> &handlers);
    void Dump(int line=0);

    std::list<TimerNode> _timers;
    int _elapsed;
};

#endif
