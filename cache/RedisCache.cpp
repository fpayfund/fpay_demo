#include "RedisCache.h"
#include "logging.h"
#include "tinyxml/tinyxml.h"
#include <iostream>
#include <string>

using namespace std;
using namespace imtixml;

RedisCache::~RedisCache()
{
    for (auto client : _clientPool) {
        delete client;
    }
}

bool RedisCache::Init(const string& confPath)
{
    vector<RedisConf> confArray;
    if (!LoadConf(confPath, confArray)) {
        return false;
    }

    for (auto &conf : confArray) {
        RedisClient* client = new RedisClient();
        if (!client->Init(conf)) {
            delete client;
        }

        LOG_DEBUG << "redis cache " << client << " connected";
        _clientPool.push_back(client);
    }

    return !_clientPool.empty();
}

RedisClient* RedisCache::Dispatch(const string& key)
{
	uint32_t hash = 1315423911;
    if (!key.empty()) {
        const char* start = key.c_str();
        const char* end = start + key.size();
        for(const char* key = start + 7; key < end; key++) {
	        hash ^= ((hash << 5) + (*key) + (hash >> 2));
	    }
    }
    hash = (hash & 0x7FFFFFFF) % _clientPool.size();

    RedisClient* client = _clientPool[hash];
    LOG_DEBUG << "dispath for key: " << key << ", hash=" << hash <<
           ", pool_size=" << _clientPool.size() << ", client=" << client;

    if (!client->IsConnected()) {
        RedisStatus status = client->Connect();
        LOG_DEBUG << "connect: " << client << ", status: " << status;
        if (status != kStatusOK) {
            return NULL;
        }
    }

    return client;
}

bool RedisCache::LoadConf(const string& confPath, std::vector<RedisConf> &confArray)
{
    if (confPath.empty()) {
        return false;
    }

    FILE* fp = fopen(confPath.c_str(), "r");
    if (!fp) {
        LOG_ERROR<< "open file fail: " << confPath;
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docHandle(&doc);
    TiXmlHandle root = docHandle.FirstChildElement( "redis_conf" );
    if (!root.Element()) {
        fclose(fp);
        return false;
    }

    TiXmlElement* serverElement = root.FirstChildElement("server").Element();
    while (serverElement) {
        RedisConf conf;
        conf._host = serverElement->Attribute("ip");
        conf._port = atoi(serverElement->Attribute("port"));
        conf._passwd = serverElement->Attribute("passwd");
        uint32_t timeout = atoi(serverElement->Attribute("timeout"));
        struct timeval tv = {timeout, 0}; 
        conf._timeout = tv;

        confArray.push_back(conf);
        serverElement = serverElement->NextSiblingElement("server");
    }    

    fclose(fp);

    return !confArray.empty();
}

void RedisCache::CheckRedis()
{
    for (auto client : _clientPool) {
        if (!client->Ping()) {
            client->Connect();
        }
    }
}

bool RedisCache::Get(const string& key, string& value)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << key;
        return false;
    }
    int ret = client->Get(key, value);
    return (ret == kStatusOK);
}

bool RedisCache::Set(const string& key, const string& value, uint32_t duration)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << key;
        return false;
    }
    int ret = client->Set(key, value, duration);
    return (ret == kStatusOK);
}

bool RedisCache::SetNX(const string& key, const string& value)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << key;
        return false;
    }

    int ret = client->Set(key, value, 0, 2);
    LOG_DEBUG << "SETNX: key=" << key << ", ret=" << ret;

    return (ret == kStatusOK);
}

bool RedisCache::Delete(const string& key)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << key;
        return false;
    }
    LOG_DEBUG << "DEL from cache: key=" << key;
    int ret = client->Delete(key);
    return (ret == kStatusOK);
}

bool RedisCache::IncrBy(const string& key, const int64_t increment, int64_t& rvalue)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << key;
        return false;
    }
    int ret = client->IncrBy(key, increment, rvalue);
    return (ret == kStatusOK);
}

bool RedisCache::enQueue(const string& topic, const std::vector<string>& vals)
{
    RedisClient* client = Dispatch(topic);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << topic;
        return false;
    }
    int ret = client->RPush(topic, vals);
    return (ret == kStatusOK);
}

bool RedisCache::enQueue(const string& topic, const string& value)
{
    RedisClient* client = Dispatch(topic);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << topic;
        return false;
    }
    std::vector<string> v;
    v.push_back(value);
    int ret = client->RPush(topic, v);
    return (ret == kStatusOK);
}

bool RedisCache::deQueue(const string& topic, string& value)
{
    RedisClient* client = Dispatch(topic);
    if (!client) {
        LOG_ERROR << "error: dispath for key: " << topic;
        return false;
    }
    int ret = client->LPop(topic, value);
    return (ret == kStatusOK);
}

bool RedisCache::deQueueAll(const string& key, vector<string>& members)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->LRange(key, 0, -1, members);
    if (ret == kStatusOK) {
        return Delete(key);
    }
    return false;
}

bool RedisCache::AddToSet(const string& key, const string& value)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->SADD(key, value);
    LOG_DEBUG << "AddToSet: key=" << key << ", member=" << value << ", ret=" << ret;
    return (ret == kStatusOK);
}

bool RedisCache::GetMembers(const string& key, vector<string>& members)
{
    RedisClient* client = Dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->SMEMBERS(key, members);
    LOG_DEBUG << "GetMembers of " << key << ", ret: " << ret;
    return (ret == kStatusOK); 
}
