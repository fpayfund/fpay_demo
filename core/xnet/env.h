#ifndef __ENV_H__
#define __ENV_H__

#include <string>
#include <iostream>
#include <time.h>

class IOLoop;
class SignalHandler;

class env
{
public:
    static IOLoop* ioLoop()
    {
        return _loop;
    }
    static void ioLoop(IOLoop* l)
    {
        _loop = l;
    }
    static SignalHandler* signalHandler(SignalHandler* handler = NULL)
    {
        if (!_signalHandler)
            _signalHandler = handler;
        return _signalHandler;
    }

    static int SocketErrorLogLevel;
    static time_t now;
    static std::string strTime;
    static unsigned int  msec;
    static uint64_t usec;

private:
    static IOLoop* _loop;
    static SignalHandler* _signalHandler;
};
#endif
