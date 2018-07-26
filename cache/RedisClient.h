#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include "hiredis/hiredis.h"

using namespace std;

namespace redis {

typedef enum {
    kStatusOK          = 0,
    kStatusConnErr     = 1,
    kStatusProtoErr    = 2,
    kStatusKeyExistErr = 3,
    kStatusUndefineErr = 4
} RedisStatus;

typedef struct {
    std::string _host;
    uint16_t _port;
    std::string _passwd;
    struct timeval _timeout;
} RedisConf;

class RedisClient
{
public:
    RedisClient()
        : _context(NULL)
    {
    }
    ~RedisClient()
    {
        Disconnect();
    }
    bool Init(const RedisConf& conf);
    bool IsConnected()
    {
        return (_context != NULL);
    }
    bool Ping();
    RedisStatus Connect();
    RedisStatus Get(const string& key, string& value);
    RedisStatus Set(const string& key, const string& value, uint32_t duration, uint32_t x = 0);
    RedisStatus Delete(const std::string& key);
    RedisStatus IncrBy(const string& key, const int64_t increment, int64_t& rvalue);
    RedisStatus LPop(const string& key, string& value);
    RedisStatus RPush(const string& key, const vector<string>& values);
    RedisStatus LRange(const string& key, const int32_t start, const int32_t stop, vector<string>& members);
    RedisStatus SADD(const string& key, const string& value);
    RedisStatus SMEMBERS(const string& key, vector<string>& members);
    RedisStatus Exec(const string& cmd, string& msg);

private:
    void Disconnect();

    RedisConf _conf;
    redisContext* _context;

};

}

#endif
