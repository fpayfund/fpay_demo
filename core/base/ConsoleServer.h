#ifndef __CONSOLE_SERVER_H_
#define __CONSOLE_SERVER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include "TcpServer.h"
#include "ProtoConsumer.h"


class ICommandHandler
{
public:
    virtual void OnCommand(const std::vector<std::string> &vecArg, std::ostream &os) = 0;
    virtual ~ICommandHandler() {}
};


template<class TClass>
class TCommandHandler: public ICommandHandler
{
public:
	typedef void (TClass::*TMethod)(const std::vector<std::string> &,  std::ostream &);

    explicit TCommandHandler(TClass *clz, TMethod mf)
        : m_clz(clz), m_mf(mf)
    {
    }

    virtual void OnCommand(const std::vector<std::string> &vecArg,  std::ostream &os)
    {
        (m_clz->*m_mf)(vecArg, os);
    }

private:
    TClass *m_clz;
    TMethod m_mf;
};

class ConsoleServer:
	public IProtoConsumer,
	public TcpServer
{
public:
	ConsoleServer();

	~ConsoleServer();

	void OnHelp(const std::vector<std::string> &vecArg, std::ostream &os);

    template<class TClass>
    void RegisterCmdHandler(const std::string &strCmd, TClass *clz, void (TClass::*mf)(const std::vector<std::string> &,  std::ostream &))
    {
        std::map<std::string, ICommandHandler *>::iterator it = m_mapHandlers.find(strCmd);
        if (it != m_mapHandlers.end())
		{
			fprintf(stderr, "duplication command handler: %s\n", strCmd.c_str());
		}

        m_mapHandlers[strCmd] = new TCommandHandler<TClass>(clz, mf);
    }

	virtual int onData(const char *pData, size_t uSize, IConn *pConn, int nType);

	virtual void setPackLimit(uint32_t sz) {};

	void DispatchCommand(const std::vector<std::string> &vecArg, IConn *pConn);

private:
    std::map<std::string, ICommandHandler *> m_mapHandlers;
};

#endif
