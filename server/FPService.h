#ifndef __FP_SERVICE_H__
#define __FP_SERVICE_H__

#include <string>
#include <map>
#include <time.h>
#include "RouteTarget.h"
#include "RedisCache.h"
#include "FPProto.h"

using namespace std;

typedef enum {
    kNodeRoleNone = 0,
    kNodeRoleRoot = 1,
    kNodeRoleMiner = 2,
} NodeRole;

class FPService
{
public:
    FPService(const string& id)
        : _id(id)
        , _nodeRole(kNodeRoleNone)
        , _blockDB(NULL)
        , _cache(NULL) 
        , _routeTarget(NULL)
    {
    }
    ~FPService();
    bool init(const string& hostPort, IRouteTarget* routeTarget);
    void getAddress(string& address);
    bool genNewBlock(const string& blockId);
    bool queryBalance(const string& userId, int64_t& value);
    bool queryBalanceString(const string& userId, string& value);
    bool handleTransaction(Transaction& tx);
    bool queryBlockChain(const string& fromBlockId, vector<Block>& blockList);
    bool handleLogin(NodeInfo& nodeInfo);
    bool handleBlockArrived(Block& block);
    bool forward(Transaction& tx);
    void generateNewBlock();

    NodeRole nodeRole()
    {
        return _nodeRole;
    }

private:
    bool initStorage();

    bool initBalance();
    bool updateBalance(const Block* block);
    bool login();

    uint64_t timestamp()
    {
        return clock();
    }
    bool tryLock(const string& lock);
    void acquireLock(const string& lock);
    void releaseLock(const string& lock);
    bool createBlock(Block& block);
    bool genBlockId(string& blockId);
    bool broadcast(const Block& block);

    bool getBlock(const string& blockId, Block& block);
    bool getLastBlock(Block& block);
    bool getLastBlockId(string& lastBlockId);
    bool storeBlock(Block& block);
    bool syncBlockChain();
    void recieveBlock(Block& block);

    string _id;
    string _address;
    string _hostport;
    NodeRole _nodeRole;

    RedisCache* _blockDB;
    RedisCache* _cache;

    string _parentNode; // ip:port
    IRouteTarget* _routeTarget;
};

#endif
