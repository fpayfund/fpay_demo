#ifndef __FP_REQ_HANDLER_H__
#define __FP_REQ_HANDLER_H__

#include <string>

#include "RouteTarget.h"
#include "ServiceContext.h"
#include "CerealHandler.h"
#include "TimerHandler.h"
#include "FPProto.h"
#include "FPService.h"

class FPReqHandler
    : public ProtoHandlerClass
    , public IRouteTarget
{
public:   
    DECLARE_FORM_MAP;

    FPReqHandler()
        : _service(NULL)
    {}

    ~FPReqHandler()
    {
        if (_service) {
            delete _service;
        }
    }

    bool init(const std::string& serviceId, uint16_t port);
    void onNodeAction(NodeInfo* req, ConnPtr_t conn);
    void onTransaction(Transaction* req, ConnPtr_t conn);
    void onBlockArrived(Block* req, ConnPtr_t conn);
    void onQueryBalance(QueryBalance* req, ConnPtr_t conn);
    void onBlockSyncRequest(BlockSyncRequest* req, ConnPtr_t conn);
    void onBlockSyncResponse(BlockSyncResponse* req, ConnPtr_t conn);
    bool onTimer()
    {
        if (!_service)
            return false;
        _service->generateNewBlock();
        return true;
    }

private:
    FPService* _service;
    TimerHandler<FPReqHandler, &FPReqHandler::onTimer> _timer;
};

#endif
