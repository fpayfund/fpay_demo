#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <vector>
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>

using std::string;
using std::vector;

class Config
{
public:
    virtual bool Load(const char * file);
    static Config* Instance();
    virtual ~Config();
    virtual std::ostream & trace(std::ostream & os) const;

public:
    int blockSyncInterval;
    int nodeRole;
    string host;
    string initBlockId;

    string lastBlockKey;
    string dbService;
    string cacheService;
    string txPoolKey;
    string BCLock;
    string childNodesKey;

    string parentNode;

    int logLevel;
    string logPath;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::make_nvp("blockSyncInterval", blockSyncInterval), 
                cereal::make_nvp("initBlockId", initBlockId),
                cereal::make_nvp("host", host),
                cereal::make_nvp("parentNode", parentNode),
                cereal::make_nvp("childNodesKey", childNodesKey),
                cereal::make_nvp("logLevel", logLevel),
                cereal::make_nvp("logPath", logPath),
                cereal::make_nvp("dbService", dbService),
                cereal::make_nvp("cacheService", cacheService),
                cereal::make_nvp("BCLock", BCLock),
                cereal::make_nvp("txPoolKey", txPoolKey),
                cereal::make_nvp("lastBlockKey", lastBlockKey),
                cereal::make_nvp("nodeRole", nodeRole));
    }
    static bool ToString(const Config& tx, std::string& json)
    {
        std::ostringstream ss;
        try {
            cereal::JSONOutputArchive archive(ss);
            archive(tx);
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
        }

        json = ss.str();
        return !json.empty();
    }

    static bool FromString(const std::string& json, Config& tx)
    {
        std::istringstream is(json);
        try {
            cereal::JSONInputArchive archive(is);
            archive(tx);
            return true;
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
        }

        return false;
    }
    static bool FromJsonFile(const std::string& file, Config& tx)
    {
        std::ifstream ifile(file);
        try {
            cereal::JSONInputArchive archive(ifile);
            archive(tx);
            return true;
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
        }

        return false;
    }
    static bool ToJsonFile(const Config& tx, std::string& file)
    {
        std::ofstream ofile(file);
        try {
            cereal::JSONOutputArchive archive(ofile);
            archive(tx);
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
            return false;
        }

        return true;
    }

    static bool FromXmlFile(const std::string& file, Config& tx)
    {
        std::ifstream ifile(file);
        try {
            cereal::XMLInputArchive archive(ifile);
            archive(tx);
            return true;
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
        }

        return false;
    }
    static bool ToXmlFile(const Config& tx, std::string& file)
    {
        std::ofstream ofile(file);
        try {
            cereal::XMLOutputArchive archive(ofile);
            archive(tx);
        }
        catch (cereal::Exception& e) {
           // LOG(WARNING) << e.what();
            return false;
        }

        return true;
    }
protected:
    Config();

private:
    static Config* instance;
};

inline std::ostream & operator << (std::ostream& os, const Config& conf)
{
    return conf.trace(os);
}
#endif
