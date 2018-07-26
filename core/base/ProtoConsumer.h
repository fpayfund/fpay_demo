#ifndef __PROTO_HANDLE_H__
#define __PROTO_HANDLE_H__

class IConn;

struct IProtoConsumer
{
	virtual ~IProtoConsumer() {}
	virtual int onData(const char* buf, size_t sz, IConn *conn, int type = 0) = 0;

	virtual void setPackLimit(int sz) = 0;
	virtual void increaseProc() {}
};

struct IProtoConsumerAware
{
	IProtoConsumer *_protoHandler;

	IProtoConsumerAware()
        : _protoHandler(NULL)
    {}
	virtual ~IProtoConsumerAware() {}
	virtual void setProtoConsumer(IProtoConsumer *h)
    {
        _protoHandler = h;
    }
	virtual IProtoConsumer *getProtoConsumer()
    {
        return _protoHandler;
    }
};

#endif
