#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <gflags/gflags.h>
#include <libgen.h>

#include "env.h"
#include "IOLoop.h"
#include "SignalHandler.h"
#include "ServerContext.h"
#include "MultiConnManager.h"
#include "DirectWriter.h"
#include "TcpServer.h"
#include "ConnFactory.h"
#include "JsonProtoConsumer.h"
#include "logging.h"
#include "Config.h"
#include "WorkerTcpServer.h"
#include "TcpServer.h"
#include "FPReqHandler.h"

using namespace std;

DEFINE_int32(channel, -1, "master data channel");
DEFINE_int32(port, 9090, "server listen port");
DEFINE_string(id, "9090", "id");
DEFINE_string(conf, "../conf/server.conf", "server xml conf file");

static void initLog(const Config & conf, const string& serverId="log_")
{
    FLAGS_log_dir = conf.logPath;
    google::InitGoogleLogging("node_server"); 
    google::SetLogFilenameExtension(serverId.c_str());
    FLAGS_logtostderr = false;
    //FLAGS_colorlogtostderr = true;
    FLAGS_minloglevel = conf.logLevel;
    google::InstallFailureSignalHandler();
}

int main(int argc, char **argv)
{
    int port = 0;

    char* path = realpath(argv[0], NULL);
    if (!path) {
        cout << "ERROR: argv[0]: " << argv[0] << endl;
        return -1;
    }
    if (chdir(dirname(path)) == -1) {
        cout << "ERROR: chdir(" << path << ")" << endl;
        return -1;
    }
    free(path);

    google::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_id.size() == 0) {
        cout << "ERROR: server id not found" << endl;
        return -1;
    }

    Config* config = Config::Instance();
#ifdef CFG_CREAL
    if (!Config::FromXmlFile(FLAGS_conf.c_str(), *config)) {
        LOG_RELEASE << "failed to load configuration: " << FLAGS_conf;
        return -1;
    }
#else
    if (!config->Load(FLAGS_conf.c_str())) {
        LOG_RELEASE << "failed to load configuration: " << FLAGS_conf;
        return -1;
    }
#endif

    std::string &serverId = FLAGS_id;
    initLog(*config, serverId);

    LOG_RELEASE << "starting FPNodeServer: pid=" << getpid() << ", channelId=" << FLAGS_channel << ", id=" << FLAGS_id << ", " << *config;

    env::ioLoop(new IOLoop());

    env::signalHandler(new SignalHandler());
    env::signalHandler()->addSig(SIGPIPE);

    ServerContext __appContext;
    MultiConnManager __connManager;
    ConnFactory screator;
    ConnFactory ccreator;
    DirectWriter __writer;
    __connManager.setClientConnFactory(&screator);
    __connManager.setServerConnFactory(&ccreator);
    __appContext.setWriter(&__writer);
    __writer.setClientConnFactory(&screator);

    IAbstractServer* __server;
    if (FLAGS_channel > 0) {
        __server = new WorkerTcpServer(FLAGS_channel);
    } else if (FLAGS_port > 0) {
        __server = new TcpServer(FLAGS_port);
    } else {
        LOG_RELEASE << "one of the parameters(channel or port) must be specified";
        return -1;
    }

    __server->start();

    JsonProtoConsumer __jsonConsumer;
    __jsonConsumer.setAppContext(&__appContext);
    __server->setConnManager(&__connManager);
    __server->setProtoConsumer(&__jsonConsumer);
    __writer.setProtoConsumer(&__jsonConsumer);

    FPReqHandler* reqHandler = new FPReqHandler();
    __appContext.addEntry(FPReqHandler::getFormEntries(), reqHandler, reqHandler);

    if (!reqHandler->init(serverId, FLAGS_port)) {
        LOG_INFO << "FPReqHandler init fail";
        return -1;
    }

    env::signalHandler()->start();
    env::ioLoop()->run();

	return 0;
}
