#ifndef __TIMER_MGMT_H__
#define __TIMER_MGMT_H__

#include <vector>

class EventHandler;

class TimerMgmt
{
public:
    virtual ~TimerMgmt() {} 
    virtual int addTimer(int timeout, EventHandler *e) = 0; 
    virtual int checkTimer(int now, std::vector<EventHandler*> &v) = 0;
};

#endif
