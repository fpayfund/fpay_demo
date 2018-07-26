/*
 * config.cc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "Config.h"
#include "tinyxml/tinyxml.h"

using namespace imtixml;

Config* Config::instance = 0;

Config* Config::Instance()
{
    if (!instance) {
        instance = new Config();
    }
    return instance;
}

Config::Config()
     : blockSyncInterval(500)
{
}

Config::~Config()
{
}

std::ostream & Config::trace(std::ostream & os) const
{
    os << " config: "
       << "`blockSyncInterval=" << blockSyncInterval
       << "`nodeRole=" << nodeRole
       << "`host=" << host
       << "`initBlockId=" << initBlockId
       << "`lastBlockKey=" << lastBlockKey
       << "`dbService=" << dbService
       << "`cacheService=" << cacheService
       << "`txPoolKey=" << txPoolKey
       << "`BCLock=" << BCLock
       << "`childNodesKey=" << childNodesKey
       << "`parentNode=" << parentNode
       << "`logLevel=" << logLevel
       << "`logPath=" << logPath;
    return os;
}

bool Config::Load(const char *file)
{
#ifndef CFG_CEREAL
    FILE* fp = fopen(file, "r");
    if (!fp) {
        std::cout << "open file fail: " << file;
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docH(&doc);
    TiXmlHandle root = docH.FirstChildElement("server");
    if (!root.Element()) {
        fclose(fp);
        return false;
    }

    bool ret = false;

    imtixml::TiXmlElement *node = root.FirstChildElement("blockSyncInterval").Element();
    if (node && node->GetText()) {
        this->blockSyncInterval = atoi(node->GetText()); 
    }

    node = root.FirstChildElement("initBlockId").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->initBlockId = node->GetText();

    node = root.FirstChildElement("host").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->host = node->GetText();

    node = root.FirstChildElement("dbService").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->dbService = node->GetText();

    node = root.FirstChildElement("cacheService").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->cacheService = node->GetText();

    node = root.FirstChildElement("lastBlockKey").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->lastBlockKey= node->GetText();

    node = root.FirstChildElement("txPoolKey").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->txPoolKey = node->GetText();

    node = root.FirstChildElement("BCLock").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->BCLock = node->GetText();

    node = root.FirstChildElement("nodeRole").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->nodeRole = atoi(node->GetText());

    node = root.FirstChildElement("childNodesKey").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->childNodesKey = node->GetText();

    node = root.FirstChildElement("parentNode").Element();
    if (node && node->GetText()) {
        this->parentNode = node->GetText();
    }

    node = root.FirstChildElement("logLevel").Element();
    if (node && node->GetText()) {
        this->logLevel = atoi(node->GetText());
    }

    node = root.FirstChildElement("logPath").Element();
    if (node && node->GetText()) {
        this->logPath = node->GetText();
    }

    return true;
#else
    return Config::FromXmlFile(file, *this);
#endif
}
