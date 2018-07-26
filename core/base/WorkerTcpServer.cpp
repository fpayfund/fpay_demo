#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>

#include "logging.h"
#include "WorkerTcpServer.h"

using namespace std;

void WorkerTcpAcceptor::onAccept(int so, uint32_t ip, int port)
{
    DLOG_TRACE;
    _server->onAccept(so, ip, port);
}

void WorkerTcpAcceptor::start()
{
    DLOG_TRACE;
    addEvent(0, EVENT_READ);
}

bool WorkerTcpAcceptor::onRecvFd(int channel, int &fd)
{
    struct msghdr msg;
    struct iovec vec;
    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    int tmp = 0;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    vec.iov_base = (void*)&tmp;
    vec.iov_len = sizeof(tmp);
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    int* recvFd = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *recvFd = -1; 

    int ret = recvmsg(channel, &msg, 0); 

    fd = *recvFd;
    LOG_INFO << "recvmsg: msgLen=" << ret << ", fd=" << fd;

    if (ret < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == ENOMEM) {
            LOG_WARN << "recvmsg error(" << errno << ") : " << strerror(errno);
            return false;
        }   

        LOG_WARN << "recvmsg error(" << errno << ") : " << strerror(errno) << ", exit.";
        exit(-1);
    }   

    if (ret == 0) {
        LOG_INFO << "Master has exited, worker also automatically exit now";
        exit(1);
    }

    return true;
}

void WorkerTcpAcceptor::doEvent(int ev) 
{
    DLOG_TRACE;
    switch (ev)
    {
        case EVENT_READ: {
            int fd = -1;
            if (onRecvFd(socket().getsocket(), fd)) {
                struct sockaddr_in sa;
                socklen_t len = sizeof(sa);
                getpeername(fd, (struct sockaddr *)&sa, &len);
                onAccept(fd, sa.sin_addr.s_addr, sa.sin_port);
                return ;
            }
            LOG_WARN << "acceptor read error:" << strerror(errno);
        }
        break;
        case EVENT_TIMEOUT: {
            onTimeout(); 
            break;
        }
        default: {
            assert(false);
            break;
        }
    }
}

void WorkerTcpServer::onAccept(int so, uint32_t ip, int port)
{
    LOG_INFO << "Accept from from :" << addr_ntoa(ip) << ":" << port;
    _connManager->createServerSideConn(so, ip, port, getProtoConsumer(), getConnEventHandler());
}

WorkerTcpAcceptor *WorkerTcpServer::createAcceptor(int fd)
{
    return new WorkerTcpAcceptor(this, fd);
}

void WorkerTcpServer::start()
{
    LOG_INFO << " tcp acceptor fd:" << _sockPairFd;
    _acceptor  = createAcceptor(_sockPairFd);
    _acceptor->start();
}
