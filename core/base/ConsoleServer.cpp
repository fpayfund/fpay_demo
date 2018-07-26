#include "ConsoleServer.h"
#include "logging.h"

static void Split(const std::string &line, const std::string &delims, std::vector<std::string>& words)
{
    std::string::size_type bi, ei;
    bi = line.find_first_not_of(delims);
    while (bi != std::string::npos)
    {
        ei = line.find_first_of(delims, bi);
        if (ei == std::string::npos)
        {
            ei = line.length();
        }
        words.push_back( line.substr(bi, ei - bi).c_str() );
        bi = line.find_first_not_of(delims, ei);
    }
}

ConsoleServer::ConsoleServer(): TcpServer(0)
{
	RegisterCmdHandler("help", this, &ConsoleServer::OnHelp);
}

ConsoleServer::~ConsoleServer()
{
	std::map<std::string, ICommandHandler *>::iterator it;
	for (it = m_mapHandlers.begin(); it != m_mapHandlers.end(); ++it)
	{
		delete it->second;
	}
}

void ConsoleServer::OnHelp(const std::vector<std::string> &vecArg, std::ostream &os)
{
	os << "+OK list supported command\r\n";

	std::map<std::string, ICommandHandler *>::iterator it;
	for (it = m_mapHandlers.begin(); it != m_mapHandlers.end(); ++it)
	{
		os << it->first << "\r\n";
	}
}

int ConsoleServer::onData(const char *pData, size_t uSize, IConn *conn, int nType)
{   
	std::vector<std::string> vecArg;
	std::string strLine;

	size_t nRead = 0;
	bool bGotLine = false;

	//always allow from localhost
	uint32_t uPeerIp = conn->getPeerIp();
	if (uPeerIp != inet_addr("127.0.0.1"))
	{
        LOG_WARN << "not allow to connect ip:", addr_ntoa(uPeerIp).c_str();
        return -1;
	}

	for (size_t i = 0; i < uSize; ++i)
	{
		if (*(pData + i) == '\n')
		{
			nRead = i + 1;

			if (i > 0 && *(pData + i - 1) == '\r')
			{
				strLine.assign(pData, i - 1);
			}
			else
			{
				strLine.assign(pData, i);
			}
			bGotLine = true;
			break;
		}
	}

	if (!bGotLine)
	{
		return 0;
	}

	if (strLine.size() > 0)
	{
		Split(strLine, " ", vecArg);
		DispatchCommand(vecArg, conn);
	}

	return nRead;
}   

void ConsoleServer::DispatchCommand(const std::vector<std::string> &vecArg, IConn *conn)
{	
	std::ostringstream os;
	if (vecArg.size() == 0)
	{
		os << "-ERR command not support \r\n";
	}
	else
	{
		std::map<std::string, ICommandHandler *>::iterator it = m_mapHandlers.find(vecArg[0]);

		if (it == m_mapHandlers.end())
		{
			os << "-ERR command not support: " << vecArg[0] << "\r\n";
		}
		else
		{
			ICommandHandler *pHandler = it->second;
			pHandler->OnCommand(vecArg, os);
		}
	}	

	std::string str = os.str();
	conn->sendBin(str.data(), str.size(), 0);
}
