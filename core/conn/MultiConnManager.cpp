#include <iostream>
#include <stdio.h>

#include "logging.h"
#include "Sender.h"
#include "MultiConnManager.h"

using namespace std;

MultiConnManager::MultiConnManager()
    : cid(0)
{
}

MultiConnManager::~MultiConnManager()
{
    for (conn_map_t::iterator it = connMap.begin(); it != connMap.end(); ++it) {
        delete (*it).second;
    }
}

void MultiConnManager::eraseConnection(IConn *conn)
{
    conn_map_t::size_type sz = connMap.erase(conn->getConnId());
    if (sz != 0) {
        delete conn;
    }
}

void MultiConnManager::eraseConnectionById(cid_t id)
{
    conn_map_t::iterator it = connMap.find(id);
    if (it != connMap.end()) {
        eraseConnection(it->second);
    }
}

IConn *MultiConnManager::getConnById(cid_t id)
{
    conn_map_t::iterator it = connMap.find(id);
    return it == connMap.end() ? NULL : it->second;
}

bool MultiConnManager::dispatchByIds(const std::set<cid_t> &ids, Sender &sender,  uint32_t exp)
{
    sender.endPack();

    for (std::set<cid_t>::const_iterator it = ids.begin(); it!= ids.end(); ++it) {
        if (exp != (uint32_t)-1 && exp == *it) {
            continue;
        }

        IConn *conn = *it < connMap.size() ? connMap[*it] : NULL;
        if (!conn) {
            return false;
        }

        try {
            conn->sendBin(sender.header(), sender.headerSize() + sender.bodySize(), sender.getUri());
        } catch(std::exception &se) {
            conn->setEnable(false);
            lazyDelIds.insert(*it);
            LOG_WARN << "SocketError in multi dispatch ids, err:" << se.what();
        }
    }

    return true;
}

bool MultiConnManager::dispatchById(cid_t cid, Sender &sender)
{
    conn_map_t::iterator it = connMap.find(cid);
    if (it != connMap.end()) {
        try {
            (*it).second->send(sender);
            return true;
        } catch (std::exception &se){
            LOG_WARN << "dispatch by cid:" << cid 
                << " err: " << se.what() 
                << " uri:" << sender.getUri() 
                <<  " size:" << sender.bodySize();
            (*it).second->setEnable(false);
            lazyDelIds.insert(cid);
        }
    } else {
        LOG_INFO << "could not dispatch response: " << cid;
    }
    
    return false;
}

void MultiConnManager::onConnCreate(IConn *conn)
{
    conn->setConnId(++cid);
    conn->setSerialId(cid);
    connMap[conn->getConnId()] = conn;
    conn->setConnEventHandler(this);
}

