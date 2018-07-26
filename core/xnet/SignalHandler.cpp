#include <iostream>
#include <sstream>
#include "SignalHandler.h"
#include "logging.h"
using namespace std;

SignalHandler::SignalHandler()
    : _sigFd(-1)
{
    sigemptyset(&_sigMask);
}

void SignalHandler::addSig(int sig)
{
    sigaddset(&_sigMask, sig);
}

bool SignalHandler::start()
{
    if (sigprocmask(SIG_BLOCK, &_sigMask, NULL) == -1) 
        return false;

    _sigFd = signalfd(-1, &_sigMask, SFD_NONBLOCK | SFD_CLOEXEC); 
    if (_sigFd == -1) 
        return false;

    socket().attach(_sigFd);
    addEvent(0, EVENT_READ);

    return true;
}

void SignalHandler::doEvent(int event)
{
    try {
        switch (event) {
        case EVENT_READ: 
            {
                char buf[2048];
                int len = ::read(socket().getsocket(), buf, sizeof(buf));
                int offset = 0;
                do
                {
                    struct signalfd_siginfo * fdsi = (struct signalfd_siginfo*)(buf + offset);
                    int sig = fdsi->ssi_signo;
                    LOG_INFO << "process signal: " << sig;

                    std::set<SignalWatcher *>::iterator it = sigWatchers.begin();
                    for (; it != sigWatchers.end(); it ++) {
                        SignalWatcher *watcher = *it;
                        watcher->onEmit(sig);
                    }
                    offset += sizeof(struct signalfd_siginfo);
                } while (offset < len);
            }
            break;
        default: 
            {
                LOG_ERROR << "signal handler ignore event: " << event;
                break;
            }
        }
    } catch (std::exception &ex) {
        LOG_ERROR << "doEvent error: " << ex.what();
    }
}

std::ostream & SignalHandler::trace(std::ostream & os) const
{
    return os << "signalFd=" << _sigFd;
}

void SignalHandler::addEvent(int remove, int add)
{
    if (_sigFd == -1) {
        return;
    }

    if ((remove + add) != EVENT_NONE) {
        env::ioLoop()->addEventHandler(remove, add, this);
    }
}

void SignalHandler::removeEvent()
{
    if (_sigFd == -1) {
        env::ioLoop()->removeEventHandler(this);
    }
}

SignalHandler::~SignalHandler()
{
    removeEvent();
}
