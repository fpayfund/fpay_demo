#ifndef __ROUTE_TARGET_H__
#define __ROUTE_TARGET_H__

#include <string>
#include "ServiceContext.h"

class IRouteTarget : public IFormTarget
{
public:
    virtual ~IRouteTarget() {}

    virtual void answer(ConnPtr_t conn, uri_t uri, const proto::Marshallable &obj);
    virtual void route(sid_t sid, uri_t uri, const proto::Marshallable &obj); 
    virtual void sendTo(const std::string& ipport, uri_t uri, const proto::Marshallable &obj); 
};

#endif
