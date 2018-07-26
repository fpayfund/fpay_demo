#ifndef __TIME_HANDLER_H__
#define __TIME_HANDLER_H__

#include "env.h"
#include "EventHandler.h"

template <typename T>
struct bindFp 
{
    typedef bool (T::*Call)();
};

template<typename T, typename bindFp<T>::Call ptr>
class TimerHandler : public EventHandler
{
public:
    TimerHandler()
    {
        _x = NULL;
        _to = 0;
    }
    TimerHandler(T* x){
        _x = x;
        _to = 0;
    }
    void init(T* x)
    {
        _x = x;
    }
    void start(uint32_t timeout)
    {
        _to = timeout;
        addTimer(_to);
    }
    void stop()
    {
        addTimer();
    }
    void doEvent(int ev)
    {
        if(_x) {		
            if((_x->*ptr)()) addTimer(_to);
        }
    }

protected:
    T* _x;
    uint32_t _to;
};

#endif
