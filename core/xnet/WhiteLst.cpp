#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <arpa/inet.h>
#include "WhiteLst.h"

using namespace std;

void WhiteLst::add(uint32_t h1, uint32_t h2) 
{
    if (!valid_addr(h1) || !valid_addr(h2))
        return;

    uint32_t i1 = ntohl(h1);
    uint32_t i2 = ntohl(h2);

    if (i2 - i1 > 64 * 1024) return; // guard

    for (uint32_t i = i1; i <= i2; ++i)
        m_hosts.insert(htonl(i));
}

void WhiteLst::parse_add(const std::string & host)
{
    const char * h = host.c_str();
    const char * c = strpbrk(h, "/-");

    if (!c) {
        // not found
        add(aton_addr(h));
        return;
    }   

    uint32_t ip1 = aton_addr(host.substr(0, c - h));

    if (!valid_addr(ip1))
        return;

    switch (*c)
    {   
        case '/':
            {   
                int len = atoi(c+1);
                if (len <= 0 || len > 32) 
                    return;
                uint32_t mask = 0x80000000;
                while (--len > 0) mask = mask >> 1 | 0x80000000;
                mask = htonl(mask);
                add(ip1 & mask, ip1 | ~mask);
            }
        case '-':
            add(ip1, aton_addr(host.substr(c - h + 1)));
            break;
    }
}

void WhiteLst::load_add(const std::string & file)
{
    std::ifstream ifile(file.c_str());
    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        add(line);
    }
}

std::ostream & WhiteLst::trace(std::ostream & os) const
{
    for (WhiteLstSet::const_iterator i = m_hosts.begin(); i != m_hosts.end(); ++i)
        os << addr_ntoa(*i) << '\n';
    return os;
}
