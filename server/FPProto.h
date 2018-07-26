#ifndef _FP_PROTOCOL_H_
#define _FP_PROTOCOL_H_

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <string>
#include <vector>
#include <map>

#include "ByteStream.h"
#include "JsonMarshallable.h"

using namespace std;
using namespace proto;

typedef enum {
    kActionLogin = 0,
    kActionLogout = 1
} NodeAction;

class QueryBalance : public proto::JsonMarshallable
{
public:
    enum { uri = 1 };

    string _address;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("addr", _address));
    }
    DECLARE_SERIALIZE
};

class NodeInfo : public proto::JsonMarshallable
{
public:
    enum { uri = 2 };

    string _address;
    string _hostport;
    uint64_t _timestamp;
    NodeAction _action;

    NodeInfo()
        : _timestamp(0)
        , _action(kActionLogin)
    {}
    NodeInfo(const string& addr, const string& hostport, uint64_t ts)
        : _address(addr)
        , _hostport(hostport)
        , _timestamp(ts)
        , _action(kActionLogin)
    {}
    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("addr", _address),
           cereal::make_nvp("hostport", _hostport),
           cereal::make_nvp("ts", _timestamp),
           cereal::make_nvp("action", _action));
    }
    DECLARE_SERIALIZE
};

class Confirmation : public proto::JsonMarshallable
{
public:
    enum { uri = 3 };

    string _address;
    uint64_t _timestamp;

    Confirmation(const string& addr, uint64_t ts)
        : _address(addr)
        , _timestamp(ts)
    {}

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("addr", _address), cereal::make_nvp("ts", _timestamp));
    }

    DECLARE_SERIALIZE
};

class Transaction : public proto::JsonMarshallable
{
public:
    enum { uri = 4 };

    string _id;
    string _from;
    string _to;
    int64_t _value;
    uint64_t _timestamp;
    //vector<Confirm> _confirmList;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("id", _id), 
           cereal::make_nvp("from", _from),
           cereal::make_nvp("to", _to), 
           cereal::make_nvp("value", _value), 
           cereal::make_nvp("ts", _timestamp));/*,
           cereal::make_nvp("confirms", _confirmList));*/
    }

    DECLARE_SERIALIZE
};

class Block : public proto::JsonMarshallable
{
public:
    enum { uri = 5 };

    string _id;
    string _prev;
    string _next;
    uint64_t _timestamp;
    vector<Transaction> _txList;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("id", _id),
           cereal::make_nvp("prev", _prev),
           cereal::make_nvp("next", _next),
           cereal::make_nvp("ts", _timestamp),
           cereal::make_nvp("txs", _txList));
    }
    DECLARE_SERIALIZE
};

class Response : public proto::JsonMarshallable
{
public:
    enum { uri = 6 };

    int32_t _status;
    string _address;
    string _desc;

    Response()
        : _status(0)
    {}

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("status", _status));
        ar(cereal::make_nvp("addr", _address));
        ar(cereal::make_nvp("desc", _desc));
    }
    DECLARE_SERIALIZE
};

class BlockSyncRequest : public proto::JsonMarshallable
{
public:
    enum { uri = 7 };

    string _address;
    string _lastBlockId;
    uint64_t _timestamp;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("address", _address),
           cereal::make_nvp("lastblock", _lastBlockId),
           cereal::make_nvp("ts", _timestamp));
    }
    DECLARE_SERIALIZE
};

class BlockSyncResponse : public proto::JsonMarshallable
{
public:
    enum { uri = 8 };

    string _address;
    vector<Block> _blockList;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::make_nvp("address", _address),
           cereal::make_nvp("blocks", _blockList));
    }
    DECLARE_SERIALIZE
};

#endif
