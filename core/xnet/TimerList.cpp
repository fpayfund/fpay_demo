#include <iostream>
#include "TimerList.h"
#include "logging.h"
using namespace std;

/**
 * arg timeout: > 0 add  new timer
 *              < 0 del exist timer
 */
int TimerList::addTimer(int timeout, EventHandler *handler)
{
    //found exist it, and del, re add new timeout
    std::list<TimerNode>::iterator it = _timers.begin();
    for (; it != _timers.end(); it ++) {
        if (it->handler == handler) {
            if (timeout > 0 && timeout != it->expire) {
                _timers.erase(it);
                break;
            }
            if (timeout < 0) {
                _timers.erase(it);
                return 0;
            }
        }
    }

    it= _timers.begin();
    for (; it != _timers.end(); it ++) {
        if (timeout < it->expire)
            break;
    }

    _timers.insert(it, TimerNode(timeout, _elapsed, handler));

    return 0;
}

void TimerList::Dump(int line)
{
    std::list<TimerNode>::iterator it = _timers.begin();
    for (; it != _timers.end(); it ++) {
        LOG_INFO << " timeout=" << it->expire << " handler=" << it->handler;
    }
    LOG_INFO << "line=" << line << " ***** timer size=" << _timers.size();
}

int TimerList::checkTimer(int tick, std::vector<EventHandler*>& handlers)
{
    _elapsed += tick;

    std::list<TimerNode>::iterator it= _timers.begin();
    for (; it != _timers.end(); ) {
        if (_elapsed - it->elapsed <= it->expire) {
            break;
        }
        handlers.push_back(it->handler);
        _timers.erase(it++);
    }

    return handlers.size();
}
