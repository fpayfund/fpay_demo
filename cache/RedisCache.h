#ifndef __REDIS_CACHE_H__
#define __REDIS_CACHE_H__

#include "RedisClient.h"
#include "hiredis/hiredis.h"

using namespace std;
using namespace redis;

class RedisCache
{
public:
    RedisCache() {}
    ~RedisCache();

    bool Init(const string& confPath);
    void CheckRedis();
    bool Get(const string& key, string& value);
    bool Set(const string& key, const string& value, uint32_t duration);
    bool SetNX(const string& key, const string& value);
    bool IncrBy(const string& key, const int64_t increment, int64_t& rvalue);
    bool Delete(const string& key);
    bool enQueue(const string& topic, const string& value);
    bool enQueue(const string& topic, const std::vector<string>& values);
    bool deQueue(const string& topic, string& value);
    bool deQueueAll(const string& topic, std::vector<string>& members);
    bool AddToSet(const string& key, const string& value);
    bool GetMembers(const string& key, vector<string>& members);

    static RedisCache* create(const string& confPath)
    {
        RedisCache* rc = new RedisCache();
        if (!rc->Init(confPath)) {
            delete rc;
            return NULL;
        }
        return rc;
    }

private:
    bool LoadConf(const string& confPath, std::vector<RedisConf> &confArray);
    RedisClient* Dispatch(const string& key);

    std::vector<RedisClient*> _clientPool;
};

#endif
