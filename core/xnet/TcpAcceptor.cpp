#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "TcpAcceptor.h"
#include "EventHandler.h"
#include "WhiteLst.h"
#include "logging.h"

using namespace std;

TcpAcceptor::TcpAcceptor(const char *ip, int port, unsigned int ops): 
    m_hosts_allow(NULL), m_ops(ops) 
{

    socket().socket();

    if (ops & SOCKOPT_REUSE)
        socket().setreuse();
    if (ops & SOCKOPT_NONBLOCK)
        socket().setblocking(false);
    if (ops & SOCKOPT_NODELAY)
        socket().setnodelay();

    socket().bind(ip, port);
    socket().listen();
}

TcpAcceptor::TcpAcceptor(int binded, unsigned int ops):
    m_hosts_allow(NULL), m_ops(ops) 
{
    socket().attach(binded);

    if (ops & SOCKOPT_REUSE)
        socket().setreuse();
    if (ops & SOCKOPT_NONBLOCK)
        socket().setblocking(false);
    if (ops & SOCKOPT_NODELAY)
        socket().setnodelay();
    if (ops & SOCKOPT_LISTEN)
        socket().listen();
}

void TcpAcceptor::doEvent(int ev) 
{
    DLOG_TRACE;
    try {
        switch (ev) {
            case EVENT_READ: 
            {
                u_long ip; 
                int port; 
                int s = -1;
                while ((s = socket().accept(&ip, &port)) != -1) {
                    if (NULL == m_hosts_allow || m_hosts_allow->find(ip)) {
                        onAccept(s, ip, port);
                        continue;
                    }
                    LOG_WARN << "access deny from=" <<  addr_ntoa(ip) << ":" <<  port;
                    close(s);
                } 
            }
            break;
        case EVENT_TIMEOUT: 
            onTimeout(); 
            break;
        default: 
            assert(false);
            break;
        }

    } catch (std::exception &ex) {
        LOG_WARN <<  "TcpAcceptor::doEvent: " << ex.what();
    }
}

void TcpAcceptor::onTimeout() 
{
    assert(false);
}
