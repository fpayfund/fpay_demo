#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gflags/gflags.h>

#include "env.h"
#include "IOLoop.h"
#include "MasterConn.h"
#include "ConnFactory.h"
#include "TcpServerMaster.h"
#include "MultiConnManager.h"
#include "logging.h"

using namespace std;

DEFINE_int32(port, 9090, "listen port");
DEFINE_int32(workers, 1, "number of worker process");
DEFINE_string(exec, "../server/node_server", "exec file path of worker process");
DEFINE_string(conf, "../conf/server.conf", "worker conf file path");
DEFINE_string(logPath, "../logs", "dir of log file");
DEFINE_int32(logLevel, 0, "log level: INFO=0 WARNNING=1 RELEASE=2");

static void initLog(const string& logPath, int logLevel)
{
    FLAGS_log_dir = logPath;
    google::InitGoogleLogging("daemon"); 
    //FLAGS_colorlogtostderr = true;
    FLAGS_logtostderr = 0;
    FLAGS_minloglevel = logLevel;
}

int main(int argc, char **argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    initLog(FLAGS_logPath, FLAGS_logLevel);

    LOG_RELEASE << "starting..........";
    while (1) {
        pid_t pid = fork();
        if (pid == 0) {
            break;
        } else if (pid > 0) {
            int status;
            pid = waitpid(-1, &status, 0); 
            LOG_INFO << "child process exit, pid=" << pid << ", status=" << status;
        } else if (pid < 0) {
            LOG_INFO << "fork error";
            return -1;
        }   
    } 

    LOG_RELEASE << "start worker";
    env::ioLoop(new IOLoop());
    env::signalHandler(new SignalHandler());
    env::signalHandler()->addSig(SIGCHLD);
    env::signalHandler()->addSig(SIGPIPE);

    TcpServerMaster __server(FLAGS_port, FLAGS_workers, FLAGS_exec.c_str(), FLAGS_conf.c_str());
    ServerSideConnFactory<MasterConn> __connFactory;
    MultiConnManager __connManager;
    __connManager.setServerConnFactory(&__connFactory);
    __server.setConnManager(&__connManager);
    __server.setSignalHandler(env::signalHandler());
    __server.start();

    env::signalHandler()->start();
    LOG_INFO << "daemon server is running";

    env::ioLoop()->run();

	return 0;
}
