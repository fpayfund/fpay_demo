#ifndef __JSON_MARSHAL_H__
#define __JSON_MARSHAL_H__

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "ByteStream.h"

namespace proto
{

struct CEStreamExn : public std::runtime_error
{
    CEStreamExn(const std::string & w)
        : std::runtime_error(w)
    {}
};

struct CEWriteStreamExn : public CEStreamExn
{
    CEWriteStreamExn(const std::string & w)
        : CEStreamExn(w)
    {}
};

struct CEReadStreamExn : public CEStreamExn
{
    CEReadStreamExn(const std::string & w)
        : CEStreamExn(w)
    {}
};

struct JsonMarshallable : public Marshallable
{
#define DECLARE_SERIALIZE \
    virtual void unmarshal(const proto::BinReadStream& up) \
    { \
        std::istringstream is(std::string(up.data(), up.size())); \
        try { \
            cereal::JSONInputArchive archive(is); \
            archive(*this); \
        } \
        catch (cereal::Exception& e) { \
            throw proto::ReadStreamExn(e.what()); \
        } \
    } \
    virtual void marshal(proto::BinWriteStream& pk) const \
    { \
        std::ostringstream ss; \
        try { \
            cereal::JSONOutputArchive archive(ss); \
            archive(*this); \
        } \
        catch (cereal::Exception& e) { \
            throw WriteStreamExn(e.what()); \
        } \
        pk.writeByte(ss.str().c_str(), (size_t)ss.str().size()); \
    } \
    virtual bool toString(std::string& str) \
    { \
        std::ostringstream ss; \
        try { \
            cereal::JSONOutputArchive archive(ss); \
            archive(*this); \
        } \
        catch (cereal::Exception& e) { \
        } \
        str = ss.str(); \
        return !str.empty(); \
    } \
    virtual bool fromString(const std::string& json) \
    { \
        std::istringstream is(json); \
        try { \
            cereal::JSONInputArchive archive(is); \
            archive(*this); \
            return true; \
        } \
        catch (cereal::Exception& e) { \
        } \
        return false; \
    }
};

}

#endif
