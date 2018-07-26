#ifndef _BASE_ICONN_H_
#define _BASE_ICONN_H_

#include <set>

#include "typedef.h"
#include "ProtoConsumer.h"
#include "ConnEventHandler.h"

struct Sender;

namespace proto {
    class Marshallable;
}

class IConn
    : public IProtoConsumerAware
    , public IConnEventHandlerAware
{
public:
	IConn()
        : bEnable(true)
    {}
	virtual ~IConn() {}

	virtual void send(Sender &rsp) = 0;
	virtual void sendBin(const char *data, size_t sz, uri_t uri) = 0;
	virtual void setTimeout(int tm) = 0;

	virtual void *getData() = 0;
	virtual void setEncKey(const unsigned char *key, size_t len) = 0;

	uint32_t getPeerIp() const { return peerIp; }
	void setPeerIp(uint32_t ip) { peerIp = ip; }
	int getPeerPort() const { return peerPort; }
	cid_t getConnId() const { return id; }
	void setConnId(cid_t cid) { id = cid; }
	void setSerialId(uint32_t id) { serialId = id; }
	uint32_t getSerialId() const { return serialId; }
	void setEnable(bool be = false) { bEnable = be; }
	bool isEnable() const { return bEnable; }
	virtual bool isEncrypto() const{ return false;}
	virtual bool isInputEncrypto() const { return false;}
	virtual bool isOutputEncrypto() const {return false;}
	virtual int getLocalIp() { return 0;}
	virtual int getConnType() { return 1;}
	virtual std::string remote_addr() { return "";}
	virtual cid_t getConnId(void) { return id; }

protected:
	cid_t id;
	uint32_t peerIp;
	uint32_t serialId;
	uint16_t peerPort;
	bool bEnable;
};

typedef IConn* ConnPtr_t;
typedef const IConn* ConstConnPtr_t;

struct IConnDispatcher
{
	virtual ~IConnDispatcher() {}
	virtual bool dispatchById(cid_t cid, Sender &) = 0;
	virtual bool dispatchByIds(const std::set<uint32_t> &ids, Sender &sender, uint32_t exp) = 0;
};

struct CreateCallback
{
	virtual ~CreateCallback(){}
	virtual void onConnCreate(IConn *conn) = 0;
};

class ClientConnFactory
{
public:
	virtual ~ClientConnFactory() {}
	virtual IConn *createConnection(const std::string& ip, uint32_t port, IProtoConsumer *consumer, IConnEventHandler *handler, CreateCallback *callback) = 0;
};

class ServerConnFactory
{
public:
	virtual ~ServerConnFactory(){}
	virtual IConn *createConnection(int fd, uint32_t ip, int port, IProtoConsumer *consumer, IConnEventHandler *handler, CreateCallback *callback) = 0;
};

class IConnManager
    : public IConnDispatcher
    , public CreateCallback
{
protected:
    ClientConnFactory *_clientConnFactory;
    ServerConnFactory *_serverConnFactory;
public:
    IConnManager()
        : _clientConnFactory(NULL)
        , _serverConnFactory(NULL)
    {}
    virtual ~IConnManager() {}

    virtual void eraseConnection(IConn *conn) = 0;
    virtual void eraseConnectionById(cid_t id) = 0;

    virtual IConn *getConnById(cid_t id) = 0;
    virtual int getLocalIp() const {return 0;}
    virtual void setClientConnFactory(ClientConnFactory *factory) { _clientConnFactory = factory;}
    virtual void setServerConnFactory(ServerConnFactory *factory) { _serverConnFactory = factory;}

    virtual IConn *createClientSideConn(const std::string& ip, uint32_t port, IProtoConsumer *consumer, IConnEventHandler *handler) = 0;
    virtual IConn *createServerSideConn(int fd, uint32_t ip, int port, IProtoConsumer *consumer, IConnEventHandler *handler) = 0;
};


class IDelayDelConn
{
public:
	virtual ~IDelayDelConn() {}
	virtual void DelayDelConn(IConn *conn) = 0;
};

class IDelayDelConnAware
{
protected:
    IDelayDelConn *_delayDelConn;
public:
    virtual void setDelayDelConn(IDelayDelConn *d) { _delayDelConn = d;}
};

class IConnManagerAware
{
protected:
    IConnManager *_connManager;
public:
    IConnManagerAware(): _connManager(NULL) {}
    virtual ~IConnManagerAware() {}
    virtual void setConnManager(IConnManager *c) { _connManager = c;}
    IConnManager *getConnManager() const { return _connManager;}
};

class IConnDispatcherAware
{
protected:
    IConnDispatcher *_dispatcher;
public:
    IConnDispatcherAware() : _dispatcher(NULL) {}
    virtual ~IConnDispatcherAware() {}
    virtual void setConnDispatcher(IConnDispatcher *c) { _dispatcher = c;}
};

#endif
