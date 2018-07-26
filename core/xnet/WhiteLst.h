#ifndef _CORE_WHITELIST_H_
#define _CORE_WHITELIST_H_
#include <set>
#include "Socket.h"
#include "typedef.h"

class WhiteLst
{
    public:
        WhiteLst() { }
        WhiteLst(const std::string & file) { load_add(file); }

        void reload(const std::string & file)   { m_hosts.clear(); load_add(file); }
        void load_add(const std::string & file);
        std::ostream & trace(std::ostream & os) const;

        void add(const std::string &host) { parse_add(host); }
        bool find(uint32_t ip) { return m_hosts.find(ip) != m_hosts.end(); }

        bool empty() { return m_hosts.empty();}
    protected:
        void add(uint32_t host) { if (valid_addr(host)) m_hosts.insert(host); }
        void add(uint32_t h1, uint32_t h2);
        void parse_add(const std::string & host);
        typedef std::set<uint32_t> WhiteLstSet;
        WhiteLstSet m_hosts;
};
#endif
