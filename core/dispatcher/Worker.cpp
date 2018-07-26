#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "logging.h"
#include "Worker.h"
#include "MasterConn.h"

bool Worker::start()
{
    _status = kStatusInit;

    if (socketpair(PF_UNIX, SOCK_STREAM, 0, _channel) < 0) {
        LOG_ERROR << "socketpair fail, errno=" << errno;
        return false;
    }

    if (fcntl(_channel[0], F_SETFD, FD_CLOEXEC) == -1) {
        LOG_ERROR << "failed to set FD_CLOEXEC for channel[0], errno " << errno;
        return false;
    }

    int bufferSize = 256 << 10;
    if (setsockopt(_channel[0], SOL_SOCKET, SO_SNDBUF, (const char*)&bufferSize, sizeof(int)) == -1) {
        LOG_ERROR << "failed to set sndbuf for channel: " << _channel[0];
        return false;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(_channel[0]);
        _channel[0] = -1;
        exec();
        exit(-1);
    } else if (pid > 0) {
        _status = kStatusOK;
        LOG_DEBUG <<  "fork a worker " << pid;
        _pid = pid;
        close(_channel[1]);
        _channel[1] = -1;
    } else {
        close(_channel[0]);
        close(_channel[1]);
        _channel[0] = -1;
        _channel[1] = -1;
        _status = kStatusUnknown;
        return false;
    }

    return true;
}

bool Worker::restart(const std::string& msg)
{
    LOG_INFO << "id=" << _id << ", pid=" << _pid << ", " << msg;

    if (_status == kStatusInit || _status == kStatusUnknown) {
        LOG_ERROR << "unexpected status: " << _status;
        return false;
    }

    close(_channel[0]);
    _channel[0] = -1;

    LOG_INFO << "killing process: pid " << _pid << ", status " << _status;
    _status = kStatusUnknown;
    pid_t pid = _pid;
    _pid = -1;
    kill(pid, SIGKILL);

    if (!start()) {
        LOG_ERROR << "could not start server: id=" << _id << ", errno=" << strerror(errno);
        return false;
    }

    LOG_INFO << "server process start: id=" << _id;
    _status = kStatusOK;

    return true;
}

int Worker::exec()
{
    LOG_INFO << "exec program: pid=" << _pid << ", bin=" << _exec << ", id=" << _id;

    char* argv[8];
    char param0[256];
    char paramName1[8];
    char paramValue1[32];
    char paramName2[8];
    char paramValue2[32];
    char paramName3[128];
    char paramValue3[32];

    snprintf(param0, 256, "%s", _exec.c_str());
    sprintf(paramName1, "-channel");
    sprintf(paramValue1, "%d", _channel[1]);
    sprintf(paramName2, "%s", "-id");
    sprintf(paramValue2, "%d", _id);
    sprintf(paramName3, "%s", "-conf");
    sprintf(paramValue3, "%s", _conf.c_str());

    argv[0] = param0;
    argv[1] = paramName1;
    argv[2] = paramValue1;
    argv[3] = paramName2;
    argv[4] = paramValue2;
    argv[5] = paramName3;
    argv[6] = paramValue3;
    argv[7] = NULL;

    return execv(argv[0], argv);
}

void IWorkerManager::startWorker()
{
    LOG_INFO << " start worker num:" << _numWorker;
    for (int i = 0; i < _numWorker; i++) {
        Worker* worker = new Worker(i, _exec, _confPath);
        _workers.push_back(worker);
        if (worker->start()) {
            LOG_DEBUG << "worker server start: id=" << i;
        } else {
            LOG_ERROR << "could not start worker server: id=" << i << ", err=" << strerror(errno);
        }
    }
}

int IWorkerManager::sendFd(int channel, int fd)
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    struct iovec vec;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    int* fdptr = (int *) CMSG_DATA(cmsg);
    *fdptr = fd;

    vec.iov_base = (void*)&(fd);
    vec.iov_len = sizeof(fd);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iovlen = 1;
    msg.msg_iov = &vec;
    msg.msg_flags = 0;

    // block I/O
    //return sendmsg(channel, &msg, MSG_NOSIGNAL);
    return sendmsg(channel, &msg, 0);
}

bool IWorkerManager::dispatch(MasterConn *conn)
{
    sessionId_t sessionId = conn->peekUint32();
    if (sessionId < 0) {
        LOG_ERROR << "less 0, session=" << sessionId;
        return false;
    }

    int i = sessionId % _workers.size();
    LOG_DEBUG << "select worker: id=" << i;
    Worker* worker = _workers[i];
    LOG_DEBUG << "select worker: id=" << worker->_id << ", status=" << worker->_status;
    if (worker->_status == kStatusOK) {
        int ret = sendFd(worker->_channel[0], conn->socket().getsocket());
        if (ret <= 0) {
            LOG_ERROR << "while sending fd: errno=" << strerror(errno);
            worker->restart("send fd error");
            return false;
        }
        LOG_DEBUG << "dispatch request to worker: id=" << worker->_id << ", ret=" << ret;
        return true;
    }

    return false;
}

Worker* IWorkerManager::findWorker(pid_t pid)
{
    for (auto worker : _workers) {
        if (worker->_pid == pid) {
            return worker;
        }
    }
    return NULL;
}

void IWorkerManager::onEmit(int sig)
{
    LOG_INFO << " watcher recv sig=" << sig; 

    int loop = 3;
    while (loop --) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            LOG_INFO << "wait pid failed, pid " << pid << " status " << status;
            return;
        }   
        LOG_INFO << "wait pid return " << pid << " status " << status;
        Worker* worker = findWorker(pid);
        if (!worker) {
            LOG_INFO << "worker " << pid << " not found";
            continue;
        }   
        LOG_INFO << " worker exit: id=" << worker->_id << ", pid=" << pid;
        worker->_status = kStatusError;
        worker->restart("signal SIGCHLD");
    }
}
