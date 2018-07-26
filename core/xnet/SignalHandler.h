#ifndef _CORE_SIGNAL_HANDLER_H_
#define _CORE_SIGNAL_HANDLER_H_
#include "EventHandler.h"
#include <sys/signalfd.h> 
#include <signal.h> 
#include <unistd.h> 
#include <set> 

class SignalWatcher
{
public:
    virtual void onEmit(int sig) = 0;
};

class SignalHandler
    : public SocketHandler
{
public:
    virtual std::ostream & trace(std::ostream & os) const;
    virtual void addEvent(int remove, int add);
    virtual void removeEvent();
    virtual void doEvent(int event); 
    virtual void watch(SignalWatcher *w)
    {
        sigWatchers.insert(w);
    }
    virtual void addSig(int sig);
    virtual bool start();

    SignalHandler();
    virtual ~SignalHandler(); 

protected:
    int _sigFd;
    sigset_t _sigMask;
    std::set<SignalWatcher *> sigWatchers;
};

class SignalAware : public SignalWatcher
{
protected:
    SignalHandler *_signalHandler;

public:
    SignalAware()
        : _signalHandler(NULL)
    {}
    virtual ~SignalAware()
    {}
    void setSignalHandler(SignalHandler* handler)
    {
        _signalHandler = handler;
        _signalHandler->watch(this); 
    }
    SignalHandler *getSignalHandler()
    {
        return _signalHandler;
    }
};

#endif
