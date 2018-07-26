#ifndef __SERVER_STATUS_H__
#define __SERVER_STATUS_H__

#ifndef S_CASE_STRING_BIGIN
#define S_CASE_STRING_BIGIN(state) switch(state){
#define S_CASE_STRING(state) case state:return #state;break;
#define S_CASE_STRING_END()  default:return "Unknown";break;}
#endif

class ServerStatus
{
public:
    enum Status {
        kNull = 0,
        kInitializing = 1,
        kInitialized = 2,
        kStarting = 3,
        kRunning = 4,
        kStopping = 5,
        kStopped = 6,
    };  

    enum SubStatus {
        kSubStatusNull = 0,
        kStoppingListener = 1,
    };  

    std::string StatusToString() const
    {
        S_CASE_STRING_BIGIN(_status);
        S_CASE_STRING(kNull);
        S_CASE_STRING(kInitialized);
        S_CASE_STRING(kRunning);
        S_CASE_STRING(kStopping);
        S_CASE_STRING(kStopped);
        S_CASE_STRING_END();
    }   

    bool isRunning() const
    {
        return _status == kRunning;
    } 

    bool isStopped() const
    {
        return _status == kStopped;
    }

    bool isStopping() const
    {
        return _status == kStopping;
    }

    ServerStatus()
        : _status(kNull)
        , _substatus(kSubStatusNull)
    {}
    virtual ~ServerStatus() {}

protected:
    Status _status;
    SubStatus _substatus;
};
#endif
