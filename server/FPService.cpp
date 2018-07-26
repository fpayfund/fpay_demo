#include "FPService.h"
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "Config.h"
#include "logging.h"
#include "Helper.h"
#include "Request.h"

using namespace std;

static Config* gConfig = Config::Instance();

static unsigned int jshash(const string& id)
{
    if (id.empty())
        return 0;

    const char* start = id.c_str();
    if (start == NULL) {
        return 0;
    }

	unsigned int hash = 1315423911;
    const char* end = start + id.size();
    for(const char* key = start + 7; key < end; key++) {
        if (*key == ' ' || *key == '\r' || *key == '\t' || *key == '&')
            break;
	    hash ^= ((hash << 5) + (*key) + (hash >> 2));
	}
    return (hash & 0x7FFFFFFF);
}

FPService::~FPService()
{
}

bool FPService::init(const string& hostPort, IRouteTarget* routeTarget)
{
    LOG_RELEASE << "FPService Init....";
    _hostport = hostPort;
    _routeTarget = routeTarget;
    _parentNode = gConfig->parentNode;

    if (gConfig->nodeRole != kNodeRoleMiner &&
        gConfig->nodeRole != kNodeRoleRoot) {
        return false;
    }

    _nodeRole = (NodeRole) gConfig->nodeRole;

    if (!initStorage()) {
        return false;
    }

    releaseLock(gConfig->BCLock);

    if (_nodeRole == kNodeRoleMiner) {
        acquireLock(gConfig->BCLock);
        bool blockSync = syncBlockChain();
        releaseLock(gConfig->BCLock);
        if (!blockSync) {
            return false;
        }
    }

    if (tryLock("_INIT_BALANCE_LOCKER")) {
        initBalance();
    }

    if (_nodeRole == kNodeRoleMiner) {
        if (tryLock(gConfig->BCLock)) {
            bool loginSuccess = login();
            releaseLock(gConfig->BCLock);
            return loginSuccess;
        }
    }

    return true;
}

void FPService::getAddress(string& address)
{
    address = _address;
}

bool FPService::syncBlockChain()
{
    Block lastBlock;
    if (!getLastBlock(lastBlock) ||
        timestamp() - lastBlock._timestamp > gConfig->blockSyncInterval) {
#if 0
        if (!UpdateBlockChain(lastBlock)) {
            //Logger.Error("update block failure, ensure you network and RootNode are available.");
        }
#endif
    }
    return true;
}

bool FPService::getBlock(const string& blockId, Block& block)
{
    LOG_DEBUG << "blockId=" << blockId;

    string blockJSONStr;
    _blockDB->Get(blockId, blockJSONStr);
    return block.fromString(blockJSONStr);
}

bool FPService::getLastBlock(Block& block)
{
    return getBlock(gConfig->lastBlockKey, block);
}

bool FPService::tryLock(const string& lock)
{
    return _cache->SetNX(lock, _id);
}

void FPService::acquireLock(const string& lock)
{
    LOG_DEBUG << "acquireLock: " << lock;
    while (1) {
        if (_cache->SetNX(lock, _id)) break;
        usleep(20);
    }
}

void FPService::releaseLock(const string& lock)
{
    string id;
    _cache->Get(lock, id);

    LOG_DEBUG << "release lock=" << lock << ", lock_id=" << id << ", self_id=" << _id;

    if (id.empty() || id == _id) {
        if (!_cache->Delete(lock)) {
            LOG_ERROR << "could not release lock=" << lock << ", lock_id=" << id << ", self_id=" << _id;
        }
    }
}

bool FPService::initStorage()
{
    _blockDB = RedisCache::create(gConfig->dbService);
    if (!_blockDB) {
        LOG_ERROR << "init block db service fails";
        return false;
    }

    _cache = RedisCache::create(gConfig->cacheService);
    if (!_cache) {
        LOG_ERROR << "init cache service fails";
        return false;
    }

    return true;
}

bool FPService::initBalance()
{
    string blockId = gConfig->initBlockId;
    LOG_RELEASE << "init balance cache: initBlockId=" << blockId;

    // 同步读，反序列化
    while (!blockId.empty()) {
        Block block; 
        if (!getBlock(blockId, block)) {
            LOG_INFO << "block not exist, id=" << blockId;
            return false;
        }
        updateBalance(&block);
        blockId = block._next;
    }

    return true;
}

void FPService::recieveBlock(Block& block)
{
    LOG_RELEASE << "recieve a new block: id=" << block._id;
    return;

    acquireLock(gConfig->BCLock);
    Block lastBlock;
    if (getLastBlock(lastBlock)) {
        lastBlock._next = block._id;
        block._prev = lastBlock._id;

        // TODO: error
        storeBlock(block);
        storeBlock(lastBlock);
    }

    releaseLock(gConfig->BCLock);

    updateBalance(&block);
}

bool FPService::storeBlock(Block& block)
{
    LOG_INFO << "store block: id=" << block._id;
    string json;
    if (block.toString(json)) {
        return _blockDB->Set(block._id, json, uint32_t(-1));
    }
    return false;
}

bool FPService::updateBalance(const Block* block)
{
    std::map<string, int64_t> balanceMap;

    for (auto & tx : block->_txList) {
        balanceMap[tx._from] -= tx._value;
        balanceMap[tx._to] += tx._value;
    }

    for (auto & pair : balanceMap) {
        int64_t balance = 0; 
        _cache->IncrBy(pair.first, pair.second, balance);
        LOG_RELEASE << "update balance: address=" << pair.first << ", balance=" << balance << ", incr=" << pair.second;
    }
}

void FPService::generateNewBlock()
{
    if (_nodeRole != kNodeRoleRoot) {
        return;
    }

    if (tryLock("_BLOCK_BUILDER_LOCK_")) {
        Block block;
        if (createBlock(block)) {
            broadcast(block);
        }

        releaseLock("_BLOCK_BUILDER_LOCK_");
    }
}

bool FPService::createBlock(Block& block)
{
    LOG_DEBUG << "try to create block";

    // TODO
    Block lastBlock;
    if (!getBlock(gConfig->lastBlockKey, lastBlock)) {
        LOG_DEBUG << "last block not found";
    }

    uint64_t ts = timestamp();
    if (lastBlock._timestamp - ts > gConfig->blockSyncInterval) {
        LOG_DEBUG << "last block is too young";
        //return false;
    }

    block._timestamp = ts;

    vector<string> txList;
    if (_cache->deQueueAll(gConfig->txPoolKey, txList)) {
        if (txList.empty()) {
            LOG_DEBUG << "transaction memory-pool is empty";
            return false;
        }

        for (auto & txStr : txList) {
            Transaction tx;
            tx.fromString(txStr); 
            block._txList.push_back(tx);
        }

        genBlockId(block._id);
        block._prev = lastBlock._id;
        lastBlock._next = block._id;
        storeBlock(block);
        storeBlock(lastBlock);
        LOG_RELEASE << "New block: id=" << block._id;
        return true;
    }

    return false;
}

bool FPService::genBlockId(string& blockId)
{
    static const uint32_t sIDLen = 64;
    char buf[sIDLen] = {0};
    Helper::GetRandStr(buf, sIDLen);
    blockId = buf;
    return true;
}

bool FPService::broadcast(const Block& block)
{
    vector<string> childNodes;
    if (!_cache->GetMembers(gConfig->childNodesKey, childNodes) || childNodes.empty()) {
        LOG_DEBUG << "child node not found";
        return false;
    }

    LOG_RELEASE << "broadcasting block to " << childNodes.size() << " child nodes";
    for (auto & nodeStr : childNodes) {
        NodeInfo nodeInfo;
        if (!nodeInfo.fromString(nodeStr)) {
            LOG_ERROR << "deserialize node fails";
            continue;
        }
        LOG_RELEASE << "send block to child: " << nodeInfo._hostport;
        _routeTarget->sendTo(nodeInfo._hostport, Block::uri, block);
    }

    return true;
}

//----------------------------------------------------------------------
// message handler
// ---------------------------------------------------------------------

// 处理查询余额的请求
bool FPService::queryBalance(const string& userId, int64_t& value)
{
    LOG_RELEASE << "query balance: address=" << userId;

    string val;
    if (_cache->Get(userId, val)) {
        value = atoll(val.c_str());
        return true;
    }
    return false;
}

bool FPService::queryBalanceString(const string& userId, string& value)
{
    bool ret = _cache->Get(userId, value);
    LOG_RELEASE << "query balance: address=" << userId << ", balance=" << value;
    return ret;
}

bool FPService::handleLogin(NodeInfo& nodeInfo)
{
    LOG_RELEASE << "Child node login: " << nodeInfo._hostport;
    string jsonStr;
    if (!nodeInfo.toString(jsonStr)) {
        LOG_ERROR << "child node info error";
        return false;
    }
    LOG_INFO << "store child node to Set:" << gConfig->childNodesKey << ", member: " << nodeInfo._hostport;
    return _cache->AddToSet(gConfig->childNodesKey, jsonStr);
}

bool FPService::handleTransaction(Transaction& tx)
{
    LOG_RELEASE << "Transaction: tid=" << tx._id << ", from=" << tx._from << ", to=" << tx._to << ", value=" << tx._value;
    int64_t balance = 0;
    if (!queryBalance(tx._from, balance)) {
        LOG_DEBUG << "query balance fail";
        return false;
    }

    LOG_DEBUG << "balance of " << tx._from << " is: " << balance;

    if (_nodeRole == kNodeRoleMiner) {
        return forward(tx);
#if 0
        if (balance >= tx._value) {
            LOG_DEBUG << "confirm tx id=" << tx._id;
            Confirm confirm(_id, timestamp());
            tx._confirmList.push_back(confirm);
            return forward(tx);
        } else {
            LOG_INFO << "balance is not enought, transaction rejected: tid=" << tx._id;
            return false;
        }
#endif
    }
    else if (_nodeRole == kNodeRoleRoot) {
        if (balance >= tx._value) {
            //Confirm confirm(_id, timestamp());
            //tx._confirmList.push_back(confirm);
            // TODO: gas
            int64_t fromVal = 0;
            int64_t toVal = 0;
            _cache->IncrBy(tx._from, -tx._value, fromVal);
            _cache->IncrBy(tx._to, tx._value, toVal);
            LOG_DEBUG << "transaction: tid=" << tx._id << ", balance: " << fromVal << " -> " << toVal;
        } else {
            LOG_INFO << "balance is not enought, transaction rejected: tid=" << tx._id;
        }

        string json;
        if (!tx.toString(json)) {
            return false;
        }
        _cache->enQueue(gConfig->txPoolKey, json);
    }
    
    return true;
}

// 返回从fromBlockId开始的区块链列表
bool FPService::queryBlockChain(const string& fromBlockId, vector<Block>& blockList)
{
    LOG_RELEASE << "query block chain from: blockId=" << fromBlockId;

    string blockId;
    Block lastBlock;
    if (getBlock(fromBlockId, lastBlock)) {
        blockId = lastBlock._next;
    }

    while (!blockId.empty()) {
        Block block;
        if (!getBlock(blockId, block)) {
            break;
        }
        blockList.push_back(block);
        blockId = block._next;
    }

    return true;
}

// 处理从网络接收到的block (矿工节点）
bool FPService::handleBlockArrived(Block& block)
{
    LOG_RELEASE << "block arrived: id=" << block._id;

    if (_nodeRole != kNodeRoleMiner) {
        return false;
    }

    recieveBlock(block);
    return true;
}

bool FPService::forward(Transaction& tx)
{
    LOG_RELEASE << "forward to parent node: " << gConfig->parentNode << ", txId=" <<  tx._id;
    if (!_routeTarget || _parentNode.empty()) {
        return false;
    }
    _routeTarget->sendTo(_parentNode, Transaction::uri, tx);
    return true;
}

bool FPService::login()
{
    if (_parentNode.empty() || _hostport.empty()) {
        LOG_ERROR << "Login fails: parentNode=" << _parentNode << ", self=" << _hostport;
        return false;
    }

    LOG_RELEASE << "Login to parent node: " << gConfig->parentNode;
    NodeInfo nodeInfo(_address, _hostport, clock_t());
    _routeTarget->sendTo(_parentNode, NodeInfo::uri, nodeInfo);

    return true;
}
