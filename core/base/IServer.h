#ifndef __ISERVER_H__
#define __ISERVER_H__

#include "typedef.h"
#include "base.h"

struct IServer:	public IProtoConsumerAware
{
	virtual ~IServer() {}

	virtual void start() = 0;

	virtual sid_t getServerId() = 0;

	virtual std::string getIp() = 0;

	virtual uint16_t getPort() = 0;
};

#endif
