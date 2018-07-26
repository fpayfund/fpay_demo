#include <iostream>

#include "logging.h"
#include "Config.h"
#include "FPReqHandler.h"

using namespace std;

static Config* gConfig = Config::Instance();

BEGIN_FORM_MAP(FPReqHandler)
    J_ON_LINK(QueryBalance, &FPReqHandler::onQueryBalance) 
    J_ON_LINK(Transaction, &FPReqHandler::onTransaction) 
    J_ON_LINK(Block, &FPReqHandler::onBlockArrived) 
    J_ON_LINK(NodeInfo, &FPReqHandler::onNodeAction) 
    J_ON_LINK(BlockSyncRequest, &FPReqHandler::onBlockSyncRequest) 
    J_ON_LINK(BlockSyncResponse, &FPReqHandler::onBlockSyncResponse) 
END_FORM_MAP()

bool FPReqHandler::init(const std::string& serviceId, uint16_t port)
{
    _service = new FPService(serviceId);

    char buf[128];
    sprintf(buf, "%s:%d", gConfig->host.c_str(), port);
    string hostPort = buf;

    if (!_service->init(hostPort, this)) {
        delete _service;
        _service = NULL;
        return false;
    }

    if (_service->nodeRole() == kNodeRoleRoot) {
        _timer.init(this);
        _timer.start(Config::Instance()->blockSyncInterval);
    }

    return true;
}

void FPReqHandler::onQueryBalance(QueryBalance* req, ConnPtr_t conn)
{
    LOG_DEBUG << "query balance: address=" << req->_address;

    Response rsp;
    rsp._status = -1;

    std::string balance;
    if (_service->queryBalanceString(req->_address, balance)) {
        rsp._status = 0;
        rsp._desc = "balance = " + balance;
    }

    answer(conn, Response::uri, rsp);
}

void FPReqHandler::onTransaction(Transaction *req, ConnPtr_t conn)
{
    Response rsp;
    if (!_service->handleTransaction(*req)) {
        rsp._status = -1;
    }
    answer(conn, Response::uri, rsp);
}

void FPReqHandler::onNodeAction(NodeInfo* req, ConnPtr_t conn)
{
    Response rsp;
    if (_service->handleLogin(*req)) {
        rsp._status = 0;
    }
    answer(conn, Response::uri, rsp);
}

void FPReqHandler::onBlockArrived(Block* req, ConnPtr_t conn)
{
    Response rsp;
    if (_service->handleBlockArrived(*req)) {
        rsp._status = 0;
    }
    answer(conn, Response::uri, rsp);
}

void FPReqHandler::onBlockSyncRequest(BlockSyncRequest* req, ConnPtr_t conn)
{
    BlockSyncResponse blockSyncResponse;
    _service->getAddress(blockSyncResponse._address);
    _service->queryBlockChain(req->_lastBlockId, blockSyncResponse._blockList);
    answer(conn, BlockSyncResponse::uri, blockSyncResponse);
}

void FPReqHandler::onBlockSyncResponse(BlockSyncResponse* req, ConnPtr_t conn)
{
    Response rsp;
    for (Block block : req->_blockList) {
        if (!_service->handleBlockArrived(block)) {
            rsp._status = -1;
            break;
        }
    }
    answer(conn, Response::uri, rsp);
}
/*
bool FPReqHandler::onTimer()
{
    if (!_service) {
        return false;
    }
    _service->generateNewBlock();
    return true;
}
*/
