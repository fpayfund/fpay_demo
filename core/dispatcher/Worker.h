#ifndef __WORKER_H__
#define __WORKER_H__

#include <string>
#include "SignalHandler.h"

class MasterConn;

typedef enum {
    kStatusInit = 0,
    kStatusOK,
    kStatusError,
    kStatusUnknown,
} WorkerStatus;

class Worker
{
public:
    Worker(int id, const std::string& exec, const std::string &conf)
        : _id(id)
        , _pid(-1)
        , _status(kStatusInit)
        , _exec(exec)
        , _conf(conf)
    {}
    ~Worker()
    {}
    bool start();
    bool restart(const std::string& msg);
    int exec();

    int _id;
    int _channel[2];     
    pid_t _pid;
    WorkerStatus _status;
    std::string _exec;
    std::string _conf;
};

class IWorkerManager: public SignalAware
{
public:
    IWorkerManager()
        : _requests(0)
        , _numWorker(0)
        , _exec("")
    {}
    IWorkerManager(uint8_t numWorker, const char* exec, const char* conf)
        : _requests(0)
        , _numWorker(numWorker)
        , _exec(exec)
        , _confPath(conf)
    {}

    virtual ~IWorkerManager() {}
    virtual int sendFd(int channel, int fd);
    virtual void startWorker();
    virtual bool dispatch(MasterConn* request);
    Worker* findWorker(pid_t pid);
    virtual void onEmit(int sig);

protected:
    int32_t _requests;
    uint8_t _numWorker;
    std::string _exec;
    std::string _confPath;
    std::vector<Worker*> _workers;
};

class IWorkerManagerAware
{
public:
    IWorkerManagerAware()
        : _workerManager(NULL)
    {}
    void setWorkerManager(IWorkerManager *s)
    {
        _workerManager =s;
    }
    IWorkerManager* getWorkerManager()
    {
        return _workerManager;
    }

protected:
    IWorkerManager * _workerManager;
};
#endif
