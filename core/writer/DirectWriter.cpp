#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "JsonSender.h"
#include "logging.h"
#include "DirectWriter.h"

using namespace std;

sid_t DirectWriter::genSId(uint32_t ip, uint16_t port)
{
    sid_t high = ip;
    high = high << 32;
    sid_t low = port;
    return high | low;
}

void DirectWriter::getAddressFromSId(sid_t sid, uint32_t &ip, uint16_t &port)
{
    sid_t high  = sid >> 32;
    ip          = high;
    port        = sid;
}

void DirectWriter::getAddress(sid_t sid, string& address)
{
    uint32_t ip = sid >> 32;
    address = addr_ntoa(ip);
    char port[16] = {0};
    snprintf(port, sizeof(port) -1, "%d", sid & 0xffff);
    address += ":";
    address += port;
}

void DirectWriter::answer(ConnPtr_t conn, uri_t uri, const proto::Marshallable &obj)
{
    JsonSender s(uri, obj);
    s.endPack();
    conn->send(s);
}

void DirectWriter::route(sid_t sid, uri_t uri, const proto::Marshallable &obj)
{
    uint32_t ip =0;
    uint16_t port = 0;
    getAddressFromSId(sid, ip, port);
    dispatch(sid, ip, port, uri, obj);
}

void DirectWriter::sendTo(const string& ipport, uri_t uri, const proto::Marshallable &obj)
{
    if (ipport.empty()) {
        return;
    }
 
    std::string ip;
    int port = 0;
    size_t pos = ipport.find_first_of(':');
    if (pos > 0) {
        ip = ipport.substr(0, pos);
        port = std::stoi(ipport.substr(pos + 1));
    }
 
    if (port <= 0 || port > 65535 || ip.empty()) {
        LOG_ERROR << "remote address error: " << ipport;
        return;
    }
 
    sid_t serverId = genSId(aton_addr(ip), port);
    route(serverId, uri, obj);
}

bool DirectWriter::dispatch(sid_t serverId, uint32_t serverIP, uint16_t port, uri_t uri, const proto::Marshallable &obj)
{
    DLOG_TRACE;
    JsonSender sender(uri, obj);

    std::map<sid_t, cid_t>::iterator it = _sid2cid.find(serverId);
    if (it != _sid2cid.end()) {
        return dispatchById(it->second, sender);
    }

    LOG_TRACE << "connect to server,  svid:" << serverId << ", ip: " << addr_ntoa(serverIP) << " port:" <<  port;
    IConn *conn = connectServer(serverId, serverIP, port);
    if (!conn) {
        LOG_WARN << "create conn error, to server,  svid:" << serverId << ", ip: " << addr_ntoa(serverIP) << " port:" <<  port;
        return false;
    }

    LOG_TRACE << "connect to server,  connId " << conn->getConnId() 
              << ", serverId:" << serverId 
              << ", ip: " << addr_ntoa(serverIP) 
              << ", port:" <<  port;

    conn->send(sender);

    return true;
}

IConn *DirectWriter::connectServer(sid_t serverId, uint32_t ip, uint16_t port)
{
    IConn *conn = NULL;
    if (serverId == (sid_t)-1) {
        return NULL;
    }

    std::string ipstr = addr_ntoa(ip);
    conn = createClientSideConn(ipstr.data(), port, getProtoConsumer(), this);

    if (!conn) {
        LOG_WARN << "error connect to server " <<  serverId;
        return NULL;
    }

     _sid2cid[serverId] = conn->getConnId();

    return conn;
}

void DirectWriter::eraseConnection(IConn *conn)
{
    DLOG_TRACE;
    if (!conn) {
        return;
    }

    std::vector<sid_t> sids; 
    for(std::map<sid_t, cid_t>::iterator it = _sid2cid.begin(); it != _sid2cid.end(); ++it) {
        if(it->second == conn->getConnId()){
            sids.push_back(it->first);
        }
    } 

    for(std::vector<sid_t>::iterator it = sids.begin(); it != sids.end(); ++it) {
        _sid2cid.erase(*it);
    }

    MultiConnManager::eraseConnection(conn);
}
