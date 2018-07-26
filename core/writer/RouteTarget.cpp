#include <iostream>
#include "RouteTarget.h"
#include "DirectWriter.h"

using namespace std;

void IRouteTarget::answer(ConnPtr_t conn, uri_t uri, const proto::Marshallable &obj)
{
    DirectWriter* directWriter = (DirectWriter*)_writer;
    directWriter->answer(conn, uri, obj);
}

void IRouteTarget::route(sid_t sid, uri_t uri, const proto::Marshallable &obj)
{
    DirectWriter* directWriter = (DirectWriter*)_writer;
    directWriter->route(sid, uri, obj);
}

void IRouteTarget::sendTo(const std::string& ipport, uri_t uri, const proto::Marshallable &obj)
{
    DirectWriter* directWriter = (DirectWriter*)_writer;
    directWriter->sendTo(ipport, uri, obj);
}
